const path = require('path');
const sdk = require('./index');

const code_dict = {
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
};

const host = '10.0.61.189';
const callback = (json, fn = () => {}) => {
  const { code, index, total }= json;
  // 错误码、当前进度、总条数
  // console.log('->', json);

  if (code > 2) {
    console.log(code_dict[code]);
  } else if (code === 0) {
    console.log('sucessed!');
    fn();
  } else if (code === 1) {
    console.log(`${index}/${total}`);
  }
};

// PLU 码下载
function pluFn(filename) {
  sdk.runTask({ host, filename, type: 0x0000 }, callback);
}

// 热键下载
function keyFn(filename) {
  sdk.runTask({ host, filename, type: 0x0003 }, callback);
}

// pluFn(path.join(__dirname, 'txt/PLU.txt'));
keyFn(path.join(__dirname, 'txt/key.txt'));
