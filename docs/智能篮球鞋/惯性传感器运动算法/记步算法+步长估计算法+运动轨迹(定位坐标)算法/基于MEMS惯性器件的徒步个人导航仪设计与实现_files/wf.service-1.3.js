/**
 * 在线客服
 */
NTKF_PARAM = {
    siteid: 'kf_9358', // 企业ID，为固定值，必填
    settingid: 'kf_9358_1469523642099', // 接待组ID，为固定值，必填
    uid: "", // 用户ID，未登录可以为空，但不能给null，uid赋予的值显示到小能客户端上
    uname: "", // 用户名，未登录可以为空，但不能给null，uname赋予的值显示到小能客户端上
    isvip: '0', // 是否为vip用户，0代表非会员，1代表会员，取值显示到小能客户端上
    userlevel: '1', // 网站自定义会员级别，0-N，可根据选择判断，取值显示到小能客户端上
    erpparam: 'abc' // erpparam为erp功能的扩展字段，可选，购买erp功能后用于erp功能集成
};
function onlineService(www) {
    $.ajax({
        url: www + 'OnlineService/GetOnlineUser',
        context: document.body,
        type: 'get',
        dataType: 'jsonp',
        success: function (data) {
            console.log('data=' + data);
            window.NTKF_PARAM.uid = data.Id;
            window.NTKF_PARAM.uname = data.Name;
            console.log('NTKF_PARAM=' + window.NTKF_PARAM.uid);
            var protocol = "https:" == location.protocol ? "https://" : "http://";
            $.get(protocol + 'dl.ntalker.com/js/xn6/ntkfstat.js?siteid=kf_9358', function () {
                console.log('基础脚本加载完成，可以执行打开聊窗');
                NTKF.im_openInPageChat('kf_9358_1469523642099');
            }, 'script');
        }
    });
}