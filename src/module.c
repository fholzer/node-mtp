#include <node_api.h>
#include <napi-macros.h>
#include "libmtp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LIBMTP_mtpdevice_t *device;

NAPI_METHOD(getFile) {
  NAPI_ARGV(2)
  NAPI_ARGV_INT32(id, 0)

  NAPI_ARGV_UTF8_MALLOC(path, 1)
  int ret = LIBMTP_Get_File_To_File(device, id, path, NULL, NULL);
  free(path);
  if (ret != 0) {
    napi_throw_error(env, NULL, "Could not retrieve file");
  }

  return NULL;
}

NAPI_METHOD(getFileListing) {
  LIBMTP_file_t * files;
  napi_value arr;

  NAPI_STATUS_THROWS(napi_create_array(env, &arr));

  files = LIBMTP_Get_Filelisting_With_Callback (device, NULL, NULL);

  if (files == NULL) {
    LIBMTP_Dump_Errorstack(device);
    LIBMTP_Clear_Errorstack(device);
  } else {
    LIBMTP_file_t *file, *tmp;
    file = files;
    int i = 0;
    while (file != NULL) {
      napi_value obj, id_uint32, filename_utf8, filesize_int64, parentId_uint32, storageId_uint32, filetype_utf8;
      NAPI_STATUS_THROWS(napi_create_object(env, &obj));

      NAPI_STATUS_THROWS(napi_create_int32(env, file->item_id, &id_uint32));
      NAPI_STATUS_THROWS(napi_set_named_property(env, obj, "id", id_uint32));

      NAPI_STATUS_THROWS(napi_create_string_utf8(env, file->filename, strlen(file->filename), &filename_utf8));
      NAPI_STATUS_THROWS(napi_set_named_property(env, obj, "filename", filename_utf8));

      if (file->filesize != (uint32_t) -1) {
        // TODO: as this is a long long (64-bit) unsigned int, maybe use BigInt in future
        NAPI_STATUS_THROWS(napi_create_int64(env, file->filesize, &filesize_int64));
        NAPI_STATUS_THROWS(napi_set_named_property(env, obj, "filesize", filesize_int64));
      }

      NAPI_STATUS_THROWS(napi_create_int32(env, file->parent_id, &parentId_uint32));
      NAPI_STATUS_THROWS(napi_set_named_property(env, obj, "parentId", parentId_uint32));

      NAPI_STATUS_THROWS(napi_create_int32(env, file->storage_id, &storageId_uint32));
      NAPI_STATUS_THROWS(napi_set_named_property(env, obj, "storageId", storageId_uint32));

      char const * filetype = LIBMTP_Get_Filetype_Description(file->filetype);
      NAPI_STATUS_THROWS(napi_create_string_utf8(env, filetype, strlen(filetype), &filetype_utf8));
      NAPI_STATUS_THROWS(napi_set_named_property(env, obj, "filetype", filetype_utf8));

      NAPI_STATUS_THROWS(napi_set_element(env, arr, i, obj));

      i += 1;
      tmp = file;
      file = file->next;
      LIBMTP_destroy_file_t(tmp);
    }

    return arr;
  }

  return NULL;
}

NAPI_METHOD(release) {
  LIBMTP_Release_Device(device);
  return NULL;
}

NAPI_METHOD(attach) {

  device = LIBMTP_Get_First_Device();

  if (device == NULL) {
    napi_throw_error(env, NULL, "No devices available.");
  } else {
    printf("Connected\n");
  }

  return NULL;
}

#define extractDeviceStringProperty(func, prop) \
    cval = LIBMTP_Get_ ## func(device); \
    if(cval == NULL) { \
      nerr = napi_set_named_property(env, dobj, #prop, null_object); \
    } else { \
      NAPI_STATUS_THROWS(napi_create_string_utf8(env, cval, strlen(cval), &val)) \
      nerr = napi_set_named_property(env, dobj, #prop, val); \
      free(cval); \
    } \
    if(nerr != napi_ok) { \
      goto cleanup; \
    }

#define setDeviceStringProperty(prop, cval) \
    if(cval == NULL) { \
      nerr = napi_set_named_property(env, dobj, #prop, null_object); \
    } else { \
      NAPI_STATUS_THROWS(napi_create_string_utf8(env, cval, strlen(cval), &val)) \
      nerr = napi_set_named_property(env, dobj, #prop, val); \
    } \
    if(nerr != napi_ok) { \
      goto cleanup; \
    }

NAPI_METHOD(getDevices) {
  napi_value* arr;
  NAPI_STATUS_THROWS(napi_create_array(env, &arr));

  napi_value null_object;
  NAPI_STATUS_THROWS(napi_get_null(env, &null_object));

  LIBMTP_raw_device_t* rawdevices = NULL;
  int numrawdevices;
  LIBMTP_error_number_t err = LIBMTP_Detect_Raw_Devices(&rawdevices, &numrawdevices);
  switch(err) {
  case LIBMTP_ERROR_NO_DEVICE_ATTACHED:
    return arr;
    return 0;
  case LIBMTP_ERROR_CONNECTING:
    napi_throw_error(env, "LIBMTP_ERROR_CONNECTING", "There has been an error connecting.");
    return NULL;
  case LIBMTP_ERROR_MEMORY_ALLOCATION:
    napi_throw_error(env, "LIBMTP_ERROR_MEMORY_ALLOCATION", "Encountered a Memory Allocation Error.");
    return NULL;
  case LIBMTP_ERROR_NONE:
    break;
  case LIBMTP_ERROR_GENERAL:
  default:
    napi_throw_error(env, "LIBMTP_ERROR_GENERAL", "Unknown connection error.");
    return NULL;
  }

  const char* cerr;
  LIBMTP_mtpdevice_t *device = NULL;
  int i;
  for (i = 0; i < numrawdevices; i++) {
    napi_value dobj;
    napi_status nerr = napi_create_object(env, &dobj);
    if(nerr != napi_ok) {
      goto cleanup;
    }
    LIBMTP_devicestorage_t *storage;
    uint8_t maxbattlevel;
    uint8_t currbattlevel;
    int ret;
    napi_value val;
    char* cval;

    setDeviceStringProperty(rawVendor, rawdevices[i].device_entry.vendor);
    setDeviceStringProperty(rawProduct, rawdevices[i].device_entry.product);

    device = LIBMTP_Open_Raw_Device_Uncached(&rawdevices[i]);
    if (device == NULL) {
      fprintf(stderr, "Unable to open raw device %d\n", i);
      continue;
    }

    extractDeviceStringProperty(Manufacturername, manufacturer);
    extractDeviceStringProperty(Modelname, model);
    extractDeviceStringProperty(Friendlyname, name);
    extractDeviceStringProperty(Serialnumber, serial);
    extractDeviceStringProperty(Deviceversion, version);

    ret = LIBMTP_Get_Batterylevel(device, &maxbattlevel, &currbattlevel);
    if (ret == 0) {
      nerr = napi_create_double(env, ((float) currbattlevel/ (float) maxbattlevel), &val);
      if(nerr != napi_ok) {
        goto cleanup;
      }
      nerr = napi_set_named_property(env, dobj, "battery", val);
      if(nerr != napi_ok) {
        goto cleanup;
      }
    } else {
      // Silently ignore. Some devices does not support getting the
      // battery level.
      LIBMTP_Clear_Errorstack(device);
      nerr = napi_set_named_property(env, dobj, "battery", null_object);
    }
    LIBMTP_Release_Device(device);
    device = NULL;

    nerr = napi_set_element(env, arr, i, dobj);
    if(nerr != napi_ok) {
      goto cleanup;
    }

  }
  free(rawdevices);
  return arr;

cleanup:
  fprintf(stderr, "jumped to cleanup. something happened...\n");
  if(device != NULL) {
    LIBMTP_Release_Device(device);
  }

  if(rawdevices != NULL) {
    free(rawdevices);
  }
}

NAPI_INIT() {
  LIBMTP_Init();
  fprintf(stdout, "libmtp version: " LIBMTP_VERSION_STRING "\n\n");

  NAPI_EXPORT_FUNCTION(attach)
  NAPI_EXPORT_FUNCTION(getFile)
  NAPI_EXPORT_FUNCTION(getFileListing)
  NAPI_EXPORT_FUNCTION(release)
  NAPI_EXPORT_FUNCTION(getDevices)
}
