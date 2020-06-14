
> 开发部分

## 联系作者
- QQ:308487730@qq.com

## usage 20-06-14
- 已提供编译好的 build/Release/aclas_sdk.node 文件
- 你可能不用再编译了，直接引用 aclas_sdk.node 文件即可
```js
const sdk = require('./build/Release/aclas_sdk.node')
```

## usage
- [https://github.com/caoxiemeihao/node-addon-aclas/blob/master/test.js](https://github.com/caoxiemeihao/node-addon-aclas/blob/master/test.js)

## 踩坑记
- binding.gyp
  * `target_name` 不能用“-”，请用“_”
  * `.gyp` 文件中不能出现中午注释
  ```diff
  {
    "targets": [
      {
  -      "target_name": "aclas-sdk",
  +      "target_name": "aclas_sdk",
  -      "sources": [ "src/aclas-sdk.c" ], # 我是注释
  +      "sources": [ "src/aclas-sdk.c" ], # comment
      }
    ]
  }
  ```

- napi 多线程使用
  * 执行 .js 文件会出现下面报错，可能是在 `napi_queue_async_work` 之后使用了 `printf`
  ```bash
  $ node index.js
  Segmentation fault
  ```

  ```diff
  + printf("---- Callback thread StartJsThread. ----\n");
    napi_queue_async_work(env, addon_data->work);
  - printf("---- Callback thread StartJsThread. ----\n");
  ```

## ---- code 对照码 ----
|十进制|八进制|说明|
|----|----|----|
|0   | 0      | 正常    |
|1   | 0x0001 | 进度事件 |
|2   | 0x0002 | 手动停止 |
|256 | 0x0100 | 已初始化 |
|257 | 0x0101 | 未初始化 |
|258 | 0x0102 | 设备不存在 |
|259 | 0x0103 | 不支持的协议类型 |
|260 | 0x0104 | 该数据类型不支持此操作 |
|261 | 0x0105 | 该数据类型不支持 |
|264 | 0x0108 | 无法打开输入文件 |
|265 | 0x0109 | 字段数与内容数不匹配 |
|266 | 0x010A | 通讯数据异常 |
|267 | 0x010B | 解析数据异常 |
|268 | 0x010C | CodePage错误 |
|269 | 0x010D | 无法创建输出文件 |

## ---- 自定义错误 ----
|十进制|说明|
|-|-|
|403 | [链接超时][默认40秒] |
|404 | [LoadLibrary][未加载到AclasSDK.dll] |

## 详细开发文档请看 doc 文件夹
- Aclas Sync SDK_V2.0_CH.pdf `开发文档`
- demo.zip `C++、C#、Delphi、PowerShell实现`
- LINK66 v3.122.exe `标签称上位机软件`
- LS2X 条码标签秤.pdf `标签称使用说明`

---

> 电子称部分

## 单品资料
- 主标签号为 0 才是自动打印

