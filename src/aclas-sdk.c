#include <windows.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#define NAPI_EXPERIMENTAL
#include <node_api.h>


// ------------------------------------------------------------------
// Aclas SDK

// ---- code 对照码 ----
// 0   0      正常
// 1   0x0001 进度事件
// 2   0x0002 手动停止
// 256 0x0100 已初始化
// 257 0x0101 未初始化
// 258 0x0102 设备不存在
// 259 0x0103 不支持的协议类型
// 260 0x0104 该数据类型不支持此操作
// 261 0x0105 该数据类型不支持
// 264 0x0108 无法打开输入文件
// 265 0x0109 字段数与内容数不匹配
// 266 0x010A 通讯数据异常
// 267 0x010B 解析数据异常
// 268 0x010C CodePage错误
// 269 0x010D 无法创建输出文件
// ---- 自定义错误 ----
// 404 [LoadLibrary][未加载到AclasSDK.dll]

typedef struct AclasDevice {
	UINT32  ProtocolType;
	UINT32  Addr;
	UINT32  Port; // 5002
	UCHAR   name[16];
	UINT32  ID;
	UINT32  Version;
	BYTE    Country;
	BYTE    DepartmentID;
	BYTE    KeyType;
	UINT64  PrinterDot;
	LONG64  PrnStartDate;
	UINT32  LabelPage;
	UINT32  PrinterNo;
	USHORT  PLUStorage;
	USHORT  HotKeyCount;
	USHORT  NutritionStorage;
	USHORT  DiscountStorage;
	USHORT  Note1Storage;
	USHORT  Note2Storage;
	USHORT  Note3Storage;
	USHORT  Note4Storage;
	BYTE    stroge[177];
} DeviceInfo;

typedef struct {
	napi_async_work work;
} SdkData;

typedef bool (CALLBACK*pAclasSDKInitialize)(char *s);
typedef bool (CALLBACK*pGetDevicesInfo)(UINT32 Addr, UINT32 Port, UINT32 ProtocolType, DeviceInfo *info);
typedef void (WINAPI *FP)(UINT32 ErrorCode, UINT32 Index, UINT32 Total, char *UserData);
typedef HANDLE(CALLBACK*pAclasSDKExecTask)(UINT32 Addr, UINT32 Port, UINT32 ProtocolType, UINT32 ProceType, UINT32 DataType, char *Filename, FP fp, char *UserData);
typedef HANDLE(CALLBACK*pAclasSDKWaitForTask)(HANDLE handle);
typedef int (CALLBACK*pAclasSDKSyncExecTask)(char *Addr, UINT32 Port, UINT32 ProtocolType, UINT32 ProceType, UINT32 DataType, char *Filename);

UINT MakeHost2Dword(char *host) {
	UINT result;
	UINT a[4];
	char* p1 = NULL;
	char str[20];
	strcpy(str, host);
	p1 = strtok(str, ".");
	a[0] = atoi(p1);
	result = a[0] << 24;
	for (int i = 1; i < 4; i++)
	{
		p1 = strtok(NULL, ".");
		a[i] = atoi(p1);
		result += a[i] << ((3 - i) * 8);
	}
	return result;
}

int g_err_code = -1;    // 错误码
int g_index = -1;       // 当前进度
int g_last_index = -1;  // 当前进度-记录
int g_total = -1;       // 总数据条数
int g_timeout = 40;     // 操作电子称超时时间
time_t g_lasttime;      // 超时时间哨兵变量
char g_host[40];        // 调子称 host(char* == char 带不带 * 都能用)
char g_filename[190];   // PLU.txt 文件名(全路径) 
int g_cmd_type;         // 执行操作类型

void WINAPI onprogress(int err_code, int index, int total, char* user_data) {
	g_err_code = err_code;
	g_index = index;
	g_total = total;
	g_lasttime = time(NULL);

	switch (err_code) {
		case 0x0000:
			// printf("complete\n");
			break;
		case 0x0001:
			// printf("%d/%d\n", index, total);
		break;
	}
}

static void run_aclas_task() {
	bool state = false;
	bool ref_info = false;
	char* user_data = NULL;
	char* host = g_host; // "10.0.61.189";

	HANDLE module = LoadLibrary("dll/AclasSDK.dll"); // %d -> 4194304
	UINT32 addr = MakeHost2Dword(host); // "10.0.61.189" == 167787965
	char *filename = g_filename; // "D:\\ypsx\\node-addon-aclas\\txt\\PLU Demo.txt";
	int DataType = g_cmd_type; // 0000 PLU, 0003 热键

	if (module == NULL) {
		g_err_code = 404;
		// printf("[dll/AclasSDK.dll] was not found. -> %d\n", module);
	}
	else {
		pAclasSDKInitialize init = (pAclasSDKInitialize)GetProcAddress(module, "AclasSDK_Initialize");
		state = init(NULL); // %d -> 1

		pGetDevicesInfo getDevicesInfo = (pGetDevicesInfo)GetProcAddress(module, "AclasSDK_GetDevicesInfo"); // 
		DeviceInfo* info = (DeviceInfo*)malloc(sizeof(DeviceInfo));
		ref_info = getDevicesInfo(addr, 0, 0, info);

		pAclasSDKExecTask execTask = (pAclasSDKExecTask)GetProcAddress(module, "AclasSDK_ExecTaskA");
		pAclasSDKWaitForTask waitForTask = (pAclasSDKWaitForTask)GetProcAddress(module, "AclasSDK_WaitForTask");
		HANDLE handle = waitForTask(execTask(addr, info->Port, info->ProtocolType, 0, DataType, filename, onprogress, user_data));

		// 释放资源
		GetProcAddress(module, "AclasSDK_Finalize");
	}
}

// Aclas worker
static void ExecuteSdkWork(napi_env env, void* data) {
	// printf("---- Sdk execute work run ----\n");
	run_aclas_task();
}

static void SdkWorkComplete(napi_env env, napi_status status, void* data) {
	SdkData* sdk_data = (SdkData*)data;

	// 释放线程
	assert(napi_delete_async_work(env, sdk_data->work) == napi_ok);

	// 回收内存
	sdk_data->work = NULL;
}

static void StartSdkThread(napi_env env, napi_callback_info info) {
	SdkData* sdk_data;
	napi_value work_name;
	napi_status status;
	napi_valuetype t1;

	status = napi_create_string_utf8(env,
		"N-API Aclas SDK Thread.",
		NAPI_AUTO_LENGTH,
		&work_name);
	assert(status == napi_ok);

	status = napi_create_async_work(env,
		NULL,
		work_name,
		ExecuteSdkWork,
		SdkWorkComplete,
		sdk_data,
		&(sdk_data->work));
	assert(status == napi_ok);

	assert(napi_queue_async_work(env, sdk_data->work) == napi_ok);
}
// ------------------------------------------------------------------


typedef struct {
	napi_async_work work;
	napi_threadsafe_function tsfn;
} AddonData;

static void CallJs(napi_env env, napi_value js_cb, void* context, void* data) {
	// js_cb == addon_data->tsfn 来自 napi_call_threadsafe_function 的第一个参数
	(void)context; // 没用到的参数
	(void)data; // 来自 napi_call_threadsafe_function 的第二个参数
	napi_status status;

	if (env != NULL) {
		// 1. 下面的 napi_value == napi_value* (加不加 * 都算指针)
		// 2. 任意的 napi_xxx 函数接受形参需要 napi_value* result，传参时都要用 & 取地址
		napi_value undefined;
		napi_value code_key, code;
		napi_value index_key, index;
		napi_value total_key, total;
		napi_value js_obj;

		napi_create_string_utf8(env, "code", NAPI_AUTO_LENGTH, &code_key);
		assert(napi_create_int32(env, g_err_code, &code) == napi_ok);

		napi_create_string_utf8(env, "index", NAPI_AUTO_LENGTH, &index_key);
		assert(napi_create_int32(env, g_index, &index) == napi_ok);

		status = napi_create_string_utf8(env, "total", NAPI_AUTO_LENGTH, &total_key);
		assert(napi_create_int32(env, g_total, &total) == napi_ok);

		// 组装回调函数参数
		napi_create_object(env, &js_obj) == napi_ok;
		assert(napi_set_property(env, js_obj, code_key, code) == napi_ok);
		assert(napi_set_property(env, js_obj, index_key, index) == napi_ok);
		assert(napi_set_property(env, js_obj, total_key, total) == napi_ok);

		// 创建一个 js 的 undefined
		assert(napi_get_undefined(env, &undefined) == napi_ok);

		// env, 回调函数的 this, 要调用的 js 回调, 回调函数参数个数, 回调函数实参, 回调函数返回对象
		status = napi_call_function(env, undefined, js_cb, 1, &js_obj, NULL);
		assert(status == napi_ok);

		// 释放内存
		free(data);
	}
}

static void ExecuteJsWork(napi_env env, void* data) {
	AddonData* addon_data = (AddonData*)data;
	napi_status status;

	// 拿到线程中到 callback
	status = napi_acquire_threadsafe_function(addon_data->tsfn);
	assert(status == napi_ok);

	// 更新超时时间
	g_lasttime = time(NULL);

	while (true)
	{
		// 执行线程中的 js callback
		// assert(napi_call_threadsafe_function(addon_data->tsfn, NULL, napi_tsfn_blocking) == napi_ok);

		if (time(NULL) - g_lasttime > g_timeout) {
			// 超时
			g_err_code = 403;
		}
		if (g_err_code > 2) {
			// 报错(提前 g_err_code 执行)
			assert(napi_call_threadsafe_function(addon_data->tsfn, NULL, napi_tsfn_blocking) == napi_ok);
			break;
		}
		if (g_err_code == 0) {
			// 完成
			assert(napi_call_threadsafe_function(addon_data->tsfn, NULL, napi_tsfn_blocking) == napi_ok);
			break;
		}
		if (g_index != g_last_index) {
			// 更新
			assert(napi_call_threadsafe_function(addon_data->tsfn, NULL, napi_tsfn_blocking) == napi_ok);
			g_last_index = g_index;
		}

		Sleep(9);
	}

	// - 释放线程中的 function
	status = napi_release_threadsafe_function(addon_data->tsfn, napi_tsfn_release);
	assert(status == napi_ok);

	// printf("---- Callback thread ExecuteJsWork. ----\n");
}

static void JsWorkComplete(napi_env env, napi_status status, void* data) {
	AddonData* addon_data = (AddonData*)data;

	// - 释放线程中的 function
	status = napi_release_threadsafe_function(addon_data->tsfn, napi_tsfn_release);
	assert(status == napi_ok);

	// 删掉线程
	if (addon_data->work != NULL) {
		status = napi_delete_async_work(env, addon_data->work);
		assert(status == napi_ok);
	}

	// 释放内存
	addon_data->work = NULL;
	addon_data->tsfn = NULL;

	// printf("---- Callback thread JsWorkComplete. ----\n");
}

static void StartJsThread(napi_env env, napi_callback_info info) {
	size_t argc = 2; // 接受两个参数
	napi_value args[2];
	napi_value js_cb, work_name;
	AddonData* addon_data;
	napi_status status;
	bool has_err = false;

	// 获取 js 回调
	// assert(napi_get_cb_info(env, info, &argc, &js_cb, NULL, (void**)(&addon_data)) == napi_ok);
	assert(napi_get_cb_info(env, info, &argc, args, NULL, (void**)(&addon_data)) == napi_ok);

	// 需要两个参数
	assert(argc >= 2);
	// 多线程情况下，抛出异常不会被主线程捕获
	// napi_throw_type_error(env, NULL, "Two arguments expected.");

	// 第一个参数 json、第二个参数 function
	/* 开多个线程时，napi_typeof 会导致崩溃 Segmentation fault
	napi_valuetype js_obj, js_callback;
	assert(napi_typeof(env, args[0], &js_obj) == napi_ok);
	assert(napi_typeof(env, args[1], &js_callback) == napi_ok);
	assert(js_obj == napi_object);
	assert(js_callback == napi_function);*/

	// 取出参数
	napi_value host_key, host;
	napi_value filename_key, filename;
	napi_value type_key, type;
	assert(napi_create_string_utf8(env, "host", NAPI_AUTO_LENGTH, &host_key) == napi_ok);
	assert(napi_create_string_utf8(env, "filename", NAPI_AUTO_LENGTH, &filename_key) == napi_ok);
	assert(napi_create_string_utf8(env, "type", NAPI_AUTO_LENGTH, &type_key) == napi_ok);
	assert(napi_get_property(env, args[0], host_key, &host) == napi_ok);
	assert(napi_get_property(env, args[0], filename_key, &filename) == napi_ok);
	assert(napi_get_property(env, args[0], type_key, &type) == napi_ok);
	// &g_host == g_host 两种写法都可以
	assert(napi_get_value_string_utf8(env, host, g_host, sizeof(g_host), NULL) == napi_ok);
	assert(napi_get_value_string_utf8(env, filename, g_filename, sizeof(g_filename), NULL) == napi_ok);
	assert(napi_get_value_int32(env, type, &g_cmd_type) == napi_ok);


	// 判断是否有未正确释放的线程
	assert(addon_data->work == NULL && "Only one work item must exist at a time");

	// 给线程起个名字
	status = napi_create_string_utf8(env,
		"N-API Thread-safe Call from Async Work Item",
		NAPI_AUTO_LENGTH,
		&work_name);
	assert(status == napi_ok);

	// 创建一个可以在线程中调用的 js callback
	status = napi_create_threadsafe_function(env,
		// js_cb,
		args[1],
		NULL,
		work_name,
		0,
		1,
		NULL,
		NULL,
		NULL,
		CallJs,
		&(addon_data->tsfn));
	assert(status == napi_ok);

	// js work 句柄
	status = napi_create_async_work(env,
		NULL,
		work_name,
		ExecuteJsWork,
		JsWorkComplete,
		addon_data,
		&(addon_data->work));
	assert(status == napi_ok);

	// printf("---- Callback thread StartJsThread. ----\n"); // 这里执行没有问题

	// js work 句柄队列启动
	status = napi_queue_async_work(env, addon_data->work);
	assert(status == napi_ok);
	// printf("---- Callback thread StartJsThread. ----\n"); // 这里执行 Segmentation fault

	// 启动 Aclas SDK 下载线程
	StartSdkThread(env, info);
}

static void addon_getting_unloaded(napi_env env, void* data, void* hint) {
	AddonData* addon_data = (AddonData*)data;
	// 执行完成，检测是否释放 work
	assert(addon_data->work == NULL && "No work item in progerss at module unload");
	free(addon_data);
}

NAPI_MODULE_INIT() {
	napi_status status;

	// new AddonData()
	AddonData* addon_data = (AddonData*)malloc(sizeof(*addon_data));
	addon_data->work = NULL;

	// SdkData* sdk_data = (SdkData*)malloc(sizeof(*sdk_data));
	// sdk_data->work = NULL;

	napi_property_descriptor js_work = {
		"runTask",
		NULL,
		StartJsThread,
		// StartSdkThread,
		NULL,
		NULL,
		NULL,
		napi_default,
		addon_data,
		// sdk_data
	};

	// 导出 runTask
	status = napi_define_properties(env, exports, 1, &js_work);
	assert(status == napi_ok);

	// 挂载 exports
	status = napi_wrap(env,
		exports,
		addon_data,
		addon_getting_unloaded,
		NULL,
		NULL);
	assert(status == napi_ok);

	return exports;
}
