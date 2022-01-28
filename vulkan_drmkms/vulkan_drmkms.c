#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <libdrm/drm.h>
#include <libdrm/drm_mode.h>
#include <xf86drmMode.h>
#include <libdrm/drm_fourcc.h>

#include <libkms/libkms.h>

#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>

int dri_dev;
drmModeResPtr dri_res;
drmModeConnectorPtr dri_conn;
drmModeEncoderPtr dri_enc;
drmModeModeInfoPtr dri_mode;

drmModeConnectorPtr get_active_connector(int dri_dev,drmModeResPtr dri_res){
	drmModeConnectorPtr dri_conn;
	for (int i=0;i<dri_res->count_connectors;i++){
		dri_conn=drmModeGetConnector(dri_dev,dri_res->connectors[i]);
		if (dri_conn){
			fprintf(stderr,"Connector %d found.\n",dri_conn->connector_id);
			if (dri_conn->connection==DRM_MODE_CONNECTED&&dri_conn->count_modes>0) return dri_conn;
			drmModeFreeConnector(dri_conn);
		}
		else fprintf(stderr,"Null connector found!\n");
	}
	return NULL;
}	

drmModeEncoderPtr get_matching_encoder(int dri_dev,drmModeResPtr dri_res,drmModeConnectorPtr dri_conn){
	drmModeEncoderPtr dri_enc;
	for (int i=0;i<dri_res->count_encoders;i++){
		dri_enc=drmModeGetEncoder(dri_dev,dri_res->encoders[i]);
		if (dri_enc){
			fprintf(stderr,"Encoder %d found.\n",dri_enc->encoder_id);
			if (dri_enc->encoder_id==dri_conn->encoder_id) return dri_enc;
			drmModeFreeEncoder(dri_enc);
		}
		else fprintf(stderr,"Null encoder found!\n");
	}
	return NULL;
}

int main(int argc,char *argv[]){
	dri_dev = open("/dev/dri/card1",O_RDWR);
	dri_res = drmModeGetResources(dri_dev);

	dri_conn = get_active_connector(dri_dev,dri_res);
	if (!dri_conn){
		drmModeFreeResources(dri_res);
		close(dri_dev);
		fprintf(stderr,"No active connector found.\n");
		return -1;
	}
	dri_mode = dri_conn->modes;

	dri_enc = get_matching_encoder(dri_dev,dri_res,dri_conn);
	if (!dri_enc){
		drmModeFreeConnector(dri_conn);
		drmModeFreeResources(dri_res);
		close(dri_dev);
		fprintf(stderr,"Failed to find a matching encoder.\n");
		return -2;
	}

	struct kms_driver *kms;
	kms_create(dri_dev,&kms);
	unsigned bo_attribs[]={
		KMS_WIDTH,dri_mode->hdisplay,
		KMS_HEIGHT,dri_mode->vdisplay,
		KMS_BO_TYPE,KMS_BO_TYPE_SCANOUT_X8R8G8B8,
		KMS_TERMINATE_PROP_LIST
	};
	struct kms_bo *bo;
	kms_bo_create(kms,bo_attribs,&bo);

	uint32_t handles[4],pitches[4],offsets[4];
	kms_bo_get_prop(bo,KMS_HANDLE,handles);
	kms_bo_get_prop(bo,KMS_PITCH,pitches);
	offsets[0]=0;

	void *plane;
	kms_bo_map(bo,&plane);

	uint32_t fb_id;
//	drmModeAddFB(dri_dev,dri_mode->hdisplay,dri_mode->vdisplay,24,32,pitches,handles,&fb_id);
	drmModeAddFB2(dri_dev,dri_mode->hdisplay,dri_mode->vdisplay,DRM_FORMAT_ARGB8888,handles,pitches,offsets,&fb_id,0);
	drmModeSetCrtc(dri_dev,dri_enc->crtc_id,fb_id,0,0/*x,y*/,&(dri_conn->connector_id),1/*element count of the connector array(the previous argument)*/,dri_mode);

	for(int i=0;i<dri_mode->hdisplay*dri_mode->vdisplay;i++){
		*((int*)plane+i)=0xFFA59841;
	}

	printf("Beginning experimental vulkan rendering instance.\n");

	VkApplicationInfo appInfo;
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pNext = NULL;
	createInfo.flags = 0;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledLayerCount = 0;

uint32_t glfwExtensionCount = 0;
const char** glfwExtensions;

glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

createInfo.enabledExtensionCount = glfwExtensionCount;
createInfo.ppEnabledExtensionNames = glfwExtensions;

	printf("config successful\n");

	VkInstance instance;
	VkResult result = vkCreateInstance(&createInfo,NULL,&instance);
	printf("%d\n",result);
	getchar();

	vkDestroyInstance(instance,NULL);

	drmModeRmFB(dri_dev,fb_id);
	kms_bo_unmap(bo);
	kms_destroy(&kms);
	drmModeFreeEncoder(dri_enc);
	drmModeFreeConnector(dri_conn);
	drmModeFreeResources(dri_res);
	close(dri_dev);
	return 0;
}
