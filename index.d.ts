export interface RunTaskParams {
  /** 电子称 IP 地址 */
  host: string
  /** 下发文件绝对路径 */
  filename: string
  /** 下发指令，具体看 Aclas 文档 */
  type: number
  /** .dll 文件绝对路径 */
  dll_path: string
}

export interface ErrorCode {
  256: '已初始化',
  257: '未初始化',
  258: '设备不存在',
  259: '不支持的协议类型',
  260: '该数据类型不支持此操作',
  261: '该数据类型不支持',
  264: '无法打开输入文件',
  265: '字段数与内容数不匹配',
  266: '通讯数据异常',
  267: '解析数据异常',
  268: 'CodePage错误',
  269: '无法创建输出文件',
  404: '[LoadLibrary][未加载到AclasSDK.dll]',
  403: '[链接超时][默认40秒]',
}

export interface CallbackArgs {
  /** 状态码 */
  code: keyof ErrorCode
  /** 当前进度 */
  index: number
  /** 总条数 */
  total: number
}

export interface Aclas {
  /** 下发任务入口 */
  runTask: (params: RunTaskParams, callback: (arg0: CallbackArgs) => void) => void
}

declare const aclas: Aclas
export default aclas