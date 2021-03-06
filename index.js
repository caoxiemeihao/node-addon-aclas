const sdk = require('./build/Release/aclas_sdk.node');
// const sdk = require('./build/Debug/aclas_sdk.node');

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
// 403 [链接超时][默认40秒]

module.exports = sdk;
