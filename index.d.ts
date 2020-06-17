/** 下发命令入参 */
export interface RunTaskParams {
  /** 电子称 IP 地址 */
  host: string
  /** 下发文件绝对路径 */
  filename: string
  /** 下发指令，具体看 Aclas 文档 */
  type: number
  /** .dll 文件绝对路径 */
  dll_path: string,
  /** 超时时间(单位秒，默认40) */
  timeout?: number
}

/** 下发状态码 */
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

  0: 'sucessed',
  1: 'processing',
}

/** 下发回调 */
export interface CallbackArgs {
  /** 状态码 */
  code: keyof ErrorCode
  /** 当前进度 */
  index: number
  /** 总条数 */
  total: number
}

/** 顶尖 Aclas */
export interface Aclas {
  /** 下发任务入口 */
  runTask: (params: RunTaskParams, callback: (arg0: CallbackArgs) => void) => void
}

declare const aclas: Aclas
export default aclas

// -----------------------------------------------------------

/** 单品资料 */
export interface PluItem {
  /** 生鲜码：自增列 */
  ID: number
  /** 货号 */
  ItemCode: number
  /** 条码起始码 */
  DepartmentID?: number
  /** 类别编号 */
  GroupID?: number
  /** 名称1 */
  Name1: string
  /** 名称2 */
  Name2?: string
  /** 名称3 */
  Name3?: string
  /** 单价 */
  Price: number
  /** 重量单位，7(500g)，4(kg) */
  UnitID: 7 | 4
  /** 条码类型1(没用) */
  BarcodeType1?: number
  /** 条码类型2(有用) */
  BarcodeType2: number
}
