$(document).ready(function(){
    if(srl==1)
    {
    	saveRlod();
    }
});
function saveRlod()
{
	$.ajax({
		url: root_path+'doc.php?',
		type: 'get',
		dataType: 'html',
		async: false,
		 data: 'act=save_rl&srlid='+srlid,
		success: function(data){
			
		}
	});
}
function getCookie(m_name)
{
	var m_cValue = "";
	var m_cookies = document.cookie.split('; ');
		for (var i = 0, l = m_cookies.length; i < l; i++) {
			var m_parts = m_cookies[i].split('=');
			if (m_parts[0]== m_name) {
				if(m_parts.length>1)
				{
					m_cValue =  unescape(m_parts[1]);
				}
			}
		}
	return m_cValue;
}
	
function sendMessage()
{
 	var login_if = getCookie("cdb_login_if");
	 if(login_if==1)
	 {
		DOC88Window.win({message:'/info.php?act=send_message&username='+$("#txtPsellername").val(),width:600,height:270,title:'发私信',iframe:true});
	 }
	 else
	 {
		 DOC88Window.win({message: '/js/home/window.html?act=window_login&actMethod=message',width:600,height:270,title:'发私信',iframe:true});
	 }
}
 function search_submit(){
	keyword = document.getElementById("keyword").value;
	var regEx = /[\^\{\}\?\*\\]/g;
	keywordnew = keyword.replace(regEx, "");
	regEx = /[\[\]\(\)\/]/g;
	keywordnew = keywordnew.replace(regEx, " ");
	if(keyword != keywordnew){
		keyword = keywordnew;
		document.getElementById("keyword").value = keyword;
	}
	if(keyword == ""){
		document.getElementById("keyword").focus();
		return false;
	}
	var searchForm = document.getElementById('searchForm');
	searchForm.action='/tag/'+encodeURIComponent(document.getElementById("keyword").value);
    searchForm.target="_blank";
	return true;
}

function getPageContent(address){
    $.ajax({
        url: address,
        dataType:"html",
        success: function(msg){
            $("#" + ShowDiv).html(msg);
        }
    });
}
function getOrderPageContent(address){
    $.ajax({
        url: address,
        dataType:"address",
        async:"false",
        success: function(msg){
            $("#ajaxorderlist").html(msg);
        }
    });
}

function getmsg2(path,pcode){
    if(doccheckstate!=1)
        return false;
    $.ajax({
        url: path+'info.php',
        type: 'get',
        dataType: 'html',
        data: 't=1&act=get_msg&rootpath='+path+'&pcode='+pcode+'&ajax=1&tm='+ Math.ceil(Math.random()*10000),
        success: function(html){
            $('#commentDiv').html(html);
        }
    });
}
//删除留言
function deleteMsg(mid, pcode){
    if(confirm("您确认删除留言吗?")==false)
        return;
    var address = root_path + "docmessage.php?act=del&messageid="+mid;
    $.ajax({
        url: address,
        dataType:  'json',
        success: function(msg){
            alert(msg.message);
            getmsg2(root_path, pcode);
        }
    });
}
function replyMsg(mid, pcode){
    ShowDiv = "AjGuestBook";
    getPageContent(root_path+"docmessage.php?act=re&rootpath="+root_path+"&messageid="+mid);
}
var m_docDownloadHtml = "";
$(document).ready(function() {
	
	if(showif=="1"){
		var show_info='<button class="btncss1" onclick="return goto_renwu()">发任务</button>&nbsp;<button class="btncss1" onclick="return goto_uploadfile()">上传文档</button>';
	}else{
		var show_info='<button class="btncssdoc88"  onclick=\'window.open("/uc/usr_account.php?act=deposit");setCookie(\"cdb_deposit_money\",downloadPrice);setCookie(\"cdb_doc_url\",window.location.href);showDepositWin();return false;\';>使用其他方式充值</button>';
	}
	
	m_docDownloadHtml="<div id='bldownloadinfoAlert' style='overflow:hidden;'>"+
	     "<div  class='popup' id='oldpopup' style='overflow:hidden;'><div class='box' style='overflow:hidden;'>"+
		     "<div class='tips' style='display:none;'>"+
	     		 "<p>"+$("#downtestw").html()+"</p>"+
	   	     "</div>"+
		     "<div class='login'>"+
		           "<ul style='width:100%;'>"+
			          "<li style='height:50px;line-height:50px;padding:0px 10px;font-size:16px;'><table width='100%' height='50' border='0' cellspacing='0' cellpadding='0'><tr><td  valign='middle' style='padding-top:10px;line-height:25px;word-break:break-all;word-wrap:break-word;'><span id='blkorderinfodoc' >"+docpricetype+"</span><span id='blAlertPotions' ></span></td></tr><tr><td  valign='middle' style='padding-left:24px;padding-top:20px;line-height:25px;word-break:break-all;word-wrap:break-word;font-size:12px;' id='downloadTsLabel'></td></tr></table></li>"+
				  "<li style='height:88px;background-color:#ffffff;'></li>"+
			          "<li>"+
			            "<input type ='button' class='btncss1' style='display:none' name='depositButton' id ='depositButton' value = '充值' onclick='window.open(\"/uc/usr_account.php?act=deposit\");setCookie(\"cdb_deposit_money\",downloadPrice);setCookie(\"cdb_doc_url\",window.location.href);windowClose();' />&nbsp;<input type='button' class='btncssdoc88' id='docDownloadButton' name='docDownloadButton' value='点击下载此文档' onclick='checkDownload(0);' />&nbsp;<input type='button' class='btnlong-small-white' value='稍后再说' onclick='windowClose();' />"+
			          "</li>"+
			   "</ul>"+
	           "</div>"+
	    "</div></div>"+
	    '<div class="box" id="newpopup" style="display:none"><div class="tips" style="color:#666"><p>下载《<span style="color:red">'+$("#downtestw").html()+'</span>》还需要 <font color="red" id="gap"></font> 积分。</p></div><div class="weixin"><form><table class="tblinfo"><tr><td class="name4"><div class="doc_qrcode" id="doc_qrcode" style="background:url(http://assets.doc88.com/assets/images/ajax-loader.gif) no-repeat center"></div></td><td><dl class="guild"><dd>温馨提示：您当前可用积分为 <font color="red" id="now_count"></font>，不足以下载此文档。</dd><dd> <p class="msg">1、您可以使用微信扫描左侧二维码快速充值获取足额积分；</p><p class="msg">2、您可以选择支付宝、银行卡等方式充值获取足额积分；</p><p class="msg">3、您可以每日登录免费领取赠送给您的积分以作积分储备。</p></dd><dd style="padding: 20px 0 0;">'+show_info+'&nbsp;<button class="btnlong-small-white" onclick="windowClose();return false;">以后再说</button>&nbsp;</dd></dl> </td> </tr></table></form></div></div><style>#newpopup{padding-left: 10px; padding-right: 10px;}#newpopup .weixin .guild { min-height: 150px;border-left: solid 1px #ccc;  padding: 0 0 0 30px;  color: #666;  }#newpopup .tips{color: #666;font-size:14px;}#newpopup .weixin {padding: 10px 0 0;}#newpopup .tblinfo {width: 100%;}#newpopup .tblinfo .name4 {width: 180px;}#newpopup .tblinfo td {padding: 5px;}#newpopup .weixin .doc_qrcode {margin-left:10px;border: solid 1px #ccc; background: white; padding: 1px;  overflow: hidden;width: 146px;height: 146px; }#newpopup .weixin .guild .msg {padding-top: 10px;}</style>'+
        "</div> <style> #depositpopup.deposit-confirm {  padding: 20px;color:#666;}#depositpopup.deposit-confirm h2 {  border-bottom: 1px solid #ccc;  font-size: 16px;  padding-bottom: 10px;  margin-bottom: 10px;}#depositpopup.deposit-confirm .explain {font-size:14px; margin-bottom: 40px;}.ui-tiptext-success .ui-tiptext-icon {color: #b5de70;}.ui-tiptext-error .ui-tiptext-icon {color: #f17975;}#depositpopup.deposit-confirm .ui-tiptext {  margin-bottom: 20px;}#depositpopup.deposit-confirm strong {  font-size: 14px;}</style><div id='depositpopup'  style='display:none' class='deposit-confirm'>  <p class='explain'>请在新打开的充值页面完成充值后选择：</p>  <p class='ui-tiptext ui-tiptext-success'>    <i class='ui-tiptext-icon iconfont' title='成功'>&#xe648;</i>    <strong class='result'>充值成功</strong>    <span> ｜ 您可以选择：</span> <a href='javascript:DOC88Window.close();downloadAlertWindow(0,0);'>点击下载此文档</a>&nbsp;&nbsp;&nbsp;&nbsp;<a href='/uc/usr_account.php?act=history' target='_blank' seed='link-complete'><span>查看充值记录</span></a>&nbsp;&nbsp;&nbsp;&nbsp;<a href='/uc/usr_invoice.php?act=request_create' target='_blank' seed='link-complete'><span>索取发票</span></a>      <p class='ui-tiptext ui-tiptext-error'>    <i class='ui-tiptext-icon iconfont' title='出错'>&#xe647;</i>    <strong class='result'>充值失败</strong>    <span> ｜ 建议您选择：</span>        <button  onclick='showWXDeposit();' class='btncssdoc88'>使用微信扫码充值</button>&nbsp;&nbsp;&nbsp;&nbsp;<a href='/help/help_info_6_31.html' target='_blank' >查看充值帮助</a>&nbsp;&nbsp;&nbsp;&nbsp;<a href='/help/contact.html' target='_blank'>联系客服</a>  </p></div>";
    document.getElementById("blkinfo").style.display = 'none';
    var buyordloperator = "下载文档，请勿使用下载工具";
    if(priceFree!=1){
        buyordloperator = "清晰阅读全文或下载文档";
    }
    var howinfo_reload="<div class='lis_ju01' id='howclearid' >"+
            "<img src='"+siteurl_images+"/skin/basic/images/20050423123030135s.gif' width='14' height='11' />"+
            "<a href='"+site_url+"/help/index.html#faq' target='_blank' title='此文档需要支付积分后，才可以清晰显示，请点击`下载`按钮'>为什么看不清楚？</a>"+
        "</div>"+
        "<div class='lis_ju01' id='howviewall' >"+
            "<img src='"+siteurl_images+"/skin/basic/images/20050423123030135s.gif' width='14' height='11' />"+
            "<a href='"+site_url+"/help/index.html#faq' et='_blank' title='此文档需要支付积分后，才可以浏览全部内容，请点击`下载`按钮'>为什么只能看到部分内容？</a>"+
        "</div>"+
        "<div class='lis_ju01' id='howbuy' style='display:none'>"+
            "<img src='"+siteurl_images+"/skin/basic/images/20050423123030135s.gif' width='14' height='11' />"+
            "<a href='javascript:;' onclick='onclick_howbuy();return false;' target='_blank' title='注册>登录>兑换积分>下载'>如何下载?</a>"+
        "</div>"+
        "<div class='lis_ju01' id='howdownload' style='display:none'>"+
            "<img src='"+siteurl_images+"/skin/basic/images/20050423123030135s.gif' width='14' height='11' />"+
            "<a href='"+site_url+"/help/index.html#help2_21' target='_blank' title='下载文档操作步骤'>如何下载到本地电脑？</a>"+
        "</div>";
    document.getElementById("howinfo").innerHTML = howinfo_reload;

    var blockinfobuy_reload="<div id='blkinfopoints_buy' style='display:none'>"+
        "<b>下载步骤：</b><br>"+
        "&nbsp;1，注册成为网站会员并登录；<br>"+
        "&nbsp;2，查看帐户积分(点击'我的文档')，如果积分不足，请参考<a style='color:#1E35A5;' target='_blank' href='"+site_url+"/help/index.html#help2_25'>'如何获得积分'</a>；<br>"+
        "&nbsp;3，点击本页面的'下载'按钮；<br>"+
        "&nbsp;4，在'文档下载'页面，点击'确认'；<br>"+
        "&nbsp;5，操作成功后，"+buyordloperator+"。<br>"+
    "</div>"+
    "<div id='blklogininfo_buy' >"+
        "<div class='blkclosebtn'></div>"+
        "<div class='blkinfotitle'><b style='font-size:16px'>"+product_shortname+"</b></div>"+
        "<div id='blklogininfodoc_buy' class='blkinfodoc' ></div>"+
        "<div id='blklogininfopoints_buy' class='blkinfopoints' ></div>"+
        "<div class='blkinfobtn' >"+
            "<input type='button' class='button' id='btncss2' value='关闭'   onclick='windowClose()' />"+
        "</div>"+
    "</div>";
    document.getElementById("blkinfobuy").innerHTML = blockinfobuy_reload;
    document.getElementById("blkinfobuy").style.display = 'none';


	document.getElementById("complainInfo").style.display = "none";
	document.getElementById("complainInfo").innerHTML = "<div>\
            <div class='blkinfodoc' style='padding-top:20px;font-size:14px'>&nbsp;&nbsp;&nbsp;&nbsp;若此文档涉嫌侵害了您的权利，请参照<a style='color:#1E35A5;' target='_blank' href='"+site_url+"/help/copyright.html'>“网站版权声明”</a>，或登录网站后，在线提交<br/>举报信息。</div>\
            <div style='font-size:14px;margin-left:20px;height:auto;line-height:25px;margin-top:5px;text-align:left;color:#C30'>&nbsp;</div>\
            <div class='blkinfobtn' style='text-align:right; padding-right:40px'>\
				<input type='button' class='btncss1'  value='登录' onclick='windowLoginForComplaint()'/>\
            </div>\
        </div>";

    if(timeLimit!="" || product_pagecount<=1){
        $("#howclearid")[0].style.display = 'none';
        $("#howviewall")[0].style.display = 'none';
    }else{
        if(statePagekey == 1){
            $("#howclearid")[0].style.display = 'none';
        }else{
            $("#howviewall")[0].style.display = 'none';
        }
    }    
   if(freeDLoadable==0)
   {
	      $("#downloadpriceid").html("<font color='red'>未提供下载</font>");

   }
   else
   {
	      $("#downloadpriceid").html("下载积分：<font color='red'>"+Math.round(downloadPrice*100)+"</font>");

   }
   
   
    $stateTimeLimit = 0;// 文档阅读期限, 0-没有权限, "notimelimit"-永久租阅, "times"-限次租阅, 其他-租阅到期时间
    if(curDownloadTimes<=0){
        if(priceFree ==1 ){
            if(freeDLoadable == 2){
                $("#howbuy")[0].style.display = 'block';
            }
        }else{
            if(timeLimit==""){
                $("#howbuy")[0].style.display = 'block';
            }
        }
    }

    if(doccheckstate==1 && logined==1 && ownPublish!=1 && curDownloadTimes>=1 ){
        if(priceFree==0 || freeDLoadable==2){
            $("#howdownload")[0].style.display = 'block';
        }
    }
   

});

//***继续下载
function continueDownload()
{
	DOC88Window.setIFrame("_blank");
	 $("#form_productbuy").submit();
}
//登录
function windowLogin()
{
	if(currentDoType==0)
		DOC88Window.setIFrame("/js/home/window.html?act=window_login&actMethod=download&logintitle="+encodeURIComponent(docpricetype));
	else
		DOC88Window.setIFrame("/js/home/window.html?act=window_login&actMethod=print");
}
//注册
function windowReg()
{
	if(currentDoType==0)
		DOC88Window.setIFrame("/js/home/window.html?act=window_reg&actMethod=download");
	else
		DOC88Window.setIFrame("/js/home/window.html?act=window_reg&actMethod=print");
}
//关闭
function windowClose()
{
	DOC88Window.close();
}

function onclick_howbuy(){
    $('#blklogininfodoc_buy').html(docpricetype);
    $('#blklogininfopoints_buy').html($('#blkinfopoints_buy').html());
	DOC88Window.win({message: $('#blkinfobuy').html(),width:600,height:320,title:'如何下载',iframe:false});
}
//AJAX获得信息
function getAjaxLogContent(address){
	$.ajax({
		url: address,
		dataType:"javascript",
		success: function(msg){
			$("#ajaxloglist").html(msg);
		}
	});
}
var m_tsLabel = "";
function getDescribe(doType)
{
	var m_label1 = "您可以选择收藏此文档，通过个人中心“<a class='link' target='_blank' href='/uc/doc_manager.php?act=doc_list&state=collect'>收藏夹</a>”在线浏览此文档。";
	var m_label2 = "您可以点击按钮下载此文档，稍后可到个人中心“<a class='link' target='_blank' href='/uc/doc_manager.php?act=doc_list&state=bought'>我的下载</a>”中查看已下载的文档。";
	var m_label3 = "您可以选择点击按钮下载此文档，或到个人中心“<a class='link' target='_blank' href='/uc/doc_manager.php?act=doc_list&state=bought'>我的下载</a>”列表中下载此文档。";
			
	Viewer.m_totalPage = product_pagecount;
	var stateDLRemainTimes = $("#stateDLRemainTimes").val();
	var describe = "";
	var closeButtonType = 0;
	var deposit = 0;
	var icon = "<i class='iconfont' style='color: #F60; font-size:24px; vertical-align:-5%'>&#xe6b2;</i>";	
	if(doType==0) //下载
	{		
		if(ownPublish==1)
		{
			m_tsLabel = m_label3;
			describe="点击按钮下载此文档(<span class='link'>"+mformat+"</span>格式,共<span class='link'>"+Viewer.m_totalPage+"</span>页)。";
			return icon+describe+"||"+closeButtonType;
		}
		if(priceFree==1) //免费阅读
		{
			if(freeDLoadable==1) //免费下载
			{	
				if(logined==0)	//尚未登录
					describe="登录后可下载此文档(<span class='link'>"+mformat+"</span>格式,共<span class='link'>"+Viewer.m_totalPage+"</span>页)。";
				else	//已经登录
					describe="点击按钮下载此文档(<span class='link'>"+mformat+"</span>格式,共<span class='link'>"+Viewer.m_totalPage+"</span>页)。";
				m_tsLabel = m_label3;
			}
			else if(freeDLoadable==2) //积分下载
			{
				if(logined==0)	//尚未登录
				{
					describe = "下载此文档(<span class='link'>"+mformat+"</span>格式,共<span class='link'>"+Viewer.m_totalPage+"</span>页)需要 <font color='red'>"+Math.round(downloadPrice*100)+"</font> 积分。";
					m_tsLabel = m_label2;
				}
				else
				{
					if(stateDLRemainTimes>0)
					{
						describe="点击按钮下载此文档(<span class='link'>"+mformat+"</span>格式,共<span class='link'>"+Viewer.m_totalPage+"</span>页)。";
						m_tsLabel = m_label3;
					}
					else
					{
						if(buyerMoney-docprice<0)
						{
							deposit = 1;
						}
						if(deposit==1)
						{
							describe = "下载此文档(<span class='link'>"+mformat+"</span>格式,共<span class='link'>"+Viewer.m_totalPage+"</span>页)需要  <font color='red'>"+Math.round(downloadPrice*100)+"</font> 积分。";
							m_tsLabel = m_label2;
						}
						else
						{
							describe = "下载此文档(<span class='link'>"+mformat+"</span>格式,共<span class='link'>"+Viewer.m_totalPage+"</span>页)将会扣除 <font color='red'>"+Math.round(downloadPrice*100)+"</font> 积分。";
							m_tsLabel = m_label2;
							closeButtonType = 1;
						}
						
					}
				}
			}
			else	//为提供下载 
			{
				m_tsLabel = m_label1;
				describe="此文档(<span class='link'>"+mformat+"</span>格式,共<span class='link'>"+Viewer.m_totalPage+"</span>页)未提供下载。";
			}
		}
	}
	return icon+describe+"||"+closeButtonType;
}
function m_getDownWinWidth()
{
	var m_width = $("#downtestw").width()+80;
	if(m_width<600)
	{
	  m_width = 600;
	}
	return m_width;
}
/****下载提示
0 表示 第一次点下载提示
1 登录成功后 调用
2 购买成功后调用
doType 0.下载 1.打印
***/
var currentDoType = 0;
function downloadAlertWindow(alertType,doType)
{
	
	document.getElementById("txtDoMethod").value = doType;
	currentDoType = doType;
	var stateDLRemainTimes = $("#stateDLRemainTimes").val();
	var doTitle = "文档下载";
	if(alertType==0)
	{
	    var m_height = 270;
	    var m_width = m_getDownWinWidth();
	    if(Viewer.m_ie6)
	    {
	       m_width = m_width+20;
	    }
	    var login_if = getCookie("cdb_login_if");
		if(login_if != 1){
		   m_width = 600;
		   m_height = 270;
		}
		DOC88Window.win({message:'',height:m_height,width:m_width,title:doTitle,iframe:true});
	}
 else{
	 DOC88Window.resizeWin(m_getDownWinWidth(),270);
 }

	var docid = $('#txtPcode').val();
	$.ajax({
		url: root_path+'doc.php?',
		type: 'get',
		dataType: 'html',
		 data: 'act=ajax_getstate&doccode='+product_code+'&checkorder=1&checkmoney=1&priceFree='+priceFree+"&freeDLoadable="+freeDLoadable+"&freePrintable="+freePrintable,
		success: function(data)
		{	
	        data = eval('(' + data + ')');
			buyerMoney = data.stateBuyermoney;
			statePrintRemainTimes = data.statePrintRemainTimes;
			curPrintTimes = statePrintRemainTimes;
			$("#statePrintRemainTimes").val(statePrintRemainTimes);
			if(data.userID == member_id)
				ownPublish = 1;
			var blLoginButton = document.getElementsByName("blLoginButton");
			var blRegButton = document.getElementsByName("blRegButton");
			var stateLogined = data.stateLogined;
			stateDLRemainTimes = data.stateDLRemainTimes;
			$("#stateDLRemainTimes").val(stateDLRemainTimes);
			curDownloadTimes = stateDLRemainTimes;
			$("#blAlertPotions").html("&nbsp;");
			var buttonAndDes = getDescribe(doType);
			var closeButtonType = buttonAndDes.split("||")[1];
			var exceptionLogin = false;
			if(logined!=stateLogined||(current_memeber_id!=""&&data.userID!=current_memeber_id))
			{
				exceptionLogin = true;
			}
			logined = stateLogined;
			current_memeber_id = data.userID;
		
			DOC88Window.setMessage(m_docDownloadHtml);
			if(stateLogined!=1)
			{
				//var login_if = getCookie("cdb_logined");
				var login_if = 1;
				if(conif=="0"){
					windowLogin();
				}else{
					if(login_if){
						windowLogin();
					}else{
						download_nologin_scan();
					}
				}
				
				if(exceptionLogin)
				{
					setPageStateByData(data);
				}
				return;
			}
			
			$("#downloadTsLabel").html(m_tsLabel);
			document.getElementById("blkorderinfodoc").innerHTML = buttonAndDes.split("||")[0];
			 document.getElementById("depositButton").style.display ="none";
			if(alertType!=2)
			{
				//设置下载按钮文字
				var downloadButton = document.getElementById("docDownloadButton");
					downloadButton.value ="点击下载此文档";
					if(stateDLRemainTimes>0)
					{
						downloadButton.value ="点击下载此文档";
					}
					else
					{
						if(priceFree!=1)
						{
							
							if(timeLimit=="notimelimit")
							{
								downloadButton.value ="点击下载此文档";
							}
							else
							{
								if(downloadable==0)
								{
									downloadButton.value ="阅读";
								}
								else
								{
									downloadButton.value ="阅读/下载";
								}
								
							}
							
				
						}
					}
					downloadButton.style.display = "";
					if(ownPublish==1)
					{
						downloadButton.value ="点击下载此文档";
					}
				if(data.stateOrderChecking == 1){
					$("#blkorderinfodoc").html("<i class='iconfont' style='color: #F60; font-size:24px; vertical-align:-5%'>&#xe6b2;</i>您的下载订单正在审核中，请稍后再试。");
					$("#downloadTsLabel").html("您可以到个人中心“<a class='link' target='_blank' href='/uc/doc_manager.php?act=doc_list&state=bought'>我的下载</a>”列表中下载此文档，或“<a class='link' target='_blank' href='/help/contact.html'>联系客服</a>”。");
					
					$("#blAlertPotions").html("&nbsp;");
					$("#docDownloadButton").hide();
					document.getElementsById("docCloseButton").value ="关闭";
					stateOrderChecking = 1;
					
				}else{
				
					//如果需要 购买 的时候判断积分 情况
					if(((stateDLRemainTimes==0&&doType==0)||(statePrintRemainTimes==0&&doType==1&&freePrintable==2))&&ownPublish==0)
					{
						var stateBuyermoney = data.stateBuyermoney;
						var deposit = 0;
						if(doType==0)
						{
							if(stateBuyermoney-docprice<0){
								if(conif=="0"&&showif!='1'){
									app_scan_download();
								}else{
								DOC88Window.resizeWin(600,270);
									deposit = 1;
									var dddd=Math.ceil(docprice-stateBuyermoney);
									$("#gap").html(Math.round((docprice-stateBuyermoney)*100));
									$("#now_count").html(Math.round(stateBuyermoney*100));
									$.ajax({
										url: '/uc/usr_account.php?act=savedeposit&c=1',
										type: 'post',
										dataType: 'html',
										data: 'paymentsub=0&payment=weixin&txt_amount='+dddd+'&amount='+dddd,
										success: function(data){
											var dataObj=eval("("+data+")");
											if(dataObj.result==1)
											{	
												$.ajax({
													url: '/uc/usr_account.php?act=weixinpay&orderno='+dataObj.order+'&c=1',
													type: 'post',
													dataType: 'html',
													success: function(data){
														var dataObj1=eval("("+data+")");
														if(dataObj1.result==1)
														{	
															$.ajax({
																url:'http://assets.doc88.com/assets/uc/js/qrcode_nomargin.js',
															dataType:"script",
															cache:true,
															success:function(){
																qrcode_img(dataObj1.url,"doc_qrcode",146);
																var times=0;
																var intervalId = window.setInterval(function(){
																	$.ajax({
																		url: '/uc/usr_account.php',
																		type: 'get',
																		dataType: 'html',
																		data: 'act=result&do_type=check&order_no='+dataObj.order,
																		success: function(data){
																			var dataObj=eval("("+data+")");
																			times++;	
																			if(dataObj.result==3)
																			{	
																				window.clearInterval(intervalId);
																				if(showif=="1"){
																					checkDownload(0);
																				}else{
																					app_scan_download();
																				}
																				
																			}
																			if(times==20){
																				window.clearInterval(intervalId);
																			}
																		}
																	});
																			
																},5000);
																
															}
															})
														
														}
													}
												});
											}
										}
									});
									$("#oldpopup").hide();
									$("#newpopup").show();
									document.getElementById('ym-header-text').innerHTML="微信扫码充值兑换积分";
							}
						}
							if(deposit==1)
							{
								var tsStr = "您的积分已不足(当前可用积分为<font color='red'>"+((parseFloat(stateBuyermoney))*100).toFixed(0)+"</font>";
								
								tsStr+="),请及时充值兑换积分。";
								$("#blAlertPotions").html(tsStr);
								$("#depositButton").show();
							}
						}
						else
						{						
							if(stateBuyermoney-freePrintprice<0)
							{
								deposit = 1;
							}
							if(deposit==1)
							{
								var tsStr = "您的积分已不足(当前可用积分为<font color='red'>"+((parseFloat(stateBuyermoney))*100).toFixed(0)+"</font>";
							
								tsStr+="),请及时充值兑换积分。";
								$("#blAlertPotions").html(tsStr);
								$("#depositButton").show();
							}
						}
						if(priceFree==1 && freeDLoadable==0&&doType==0)
						{
							$("#blAlertPotions").html("&nbsp;");
						}
						if(freePrintable==0&&doType==1)
						{
							$("#blAlertPotions").html("&nbsp;");
						}
					}
				}
                		//不能下载时，隐藏下载按钮
				if(doType==0)
				{
					if((doccheckstate != 1||(priceFree==1 && freeDLoadable==0) || deposit==1 ||stateOrderChecking==1||(timeLimit=="notimelimit"&&downloadable==0))&&ownPublish==0)
					{
						//$("#docDownloadButton").hide();
						$("#docDownloadButton").val("点击收藏此文档");
						$("#docDownloadButton").attr("onclick","");
						$("#docDownloadButton").click(function(){
							windowClose();
							checkcollectproduct();
						});
					}
				}
				else
				{
					if((doccheckstate != 1||freePrintable==0|| deposit==1 ||stateOrderChecking==1)&&ownPublish==0)
					{
						$("#docDownloadButton").hide();
					}
				}	
			}			
			//是否是登录之后 点 右边的下载 直接 返回
			if(alertType!=0||exceptionLogin)
			{
           	setPageStateByData(data);
			}
			
		}
	});
}

/***重新设置页面状态****/
function reloadPageState(method)
{
	var stateDLRemainTimes = $("#stateDLRemainTimes").val();
	var docid = $('#txtPcode').val();
	$.ajax({
		url: root_path+'doc.php?',
		type: 'get',
		dataType: 'html',
		async: false,
		  data: 'act=ajax_getstate&doccode='+product_code+'&checkorder=1&checkmoney=1&priceFree='+priceFree+"&freeDLoadable="+freeDLoadable+"&freePrintable="+freePrintable,
		success: function(data){
			var data=eval("("+data+")");
			setPageStateByData(data);
		}
	});
}
/****设置setPageState***/
function setPageStateByData(data)
{
	if(data.userID == member_id)
		ownPublish = 1;
	var m_statePrintRemainTimes = data.statePrintRemainTimes;
	
	curPrintTimes = m_statePrintRemainTimes;
	$("#statePrintRemainTimes").val(m_statePrintRemainTimes);
	var stateLogined = data.stateLogined;
	logined = stateLogined;
	if(ownPublish == 1){
		data.stateTimeLimit = "notimelimit";
	}
	stateDLRemainTimes = data.stateDLRemainTimes;
	$("#stateDLRemainTimes").val(stateDLRemainTimes);
	curDownloadTimes = stateDLRemainTimes;
	if(priceFree==0&&data.stateTimeLimit!="0"&&(timeLimit=="0"||timeLimit==""||timeLimit!="notimelimit"))
	{
		timeLimit = data.stateTimeLimit;
	}
	timeLimit = data.stateTimeLimit;
	//更新页面状态
	if(data.stateTimeLimit!="0"||data.stateRemainTimes!=0)
	{
		$("#viewTitleDiv").html("");
	}
	if(data.stateRemainTimes!=0)
	{
		$("#timelimit").html("阅读期限：在线阅读"+data.stateRemainTimes+"次");
		return;
	}
	if(data.stateTimeLimit=="0")
	{	
		$("#timelimit").html("阅读期限：没有阅读权限");
	}
	else if(data.stateTimeLimit=="notimelimit")
	{
		$("#timelimit").html("阅读期限：不限期阅读");
		$("#txtPonlydownload").val("1");
	}
	else
	{
		var m_now = new Date();
		m_now.setTime(data.stateTimeLimit*1000);
		m_n = m_now.getYear();
		m_y = m_now.getMonth()+1;
		m_r = m_now.getDate();
		m_h = m_now.getHours();
		m_m =m_now.getMinutes();
		m_s = m_now.getSeconds();
		if(m_n<1900) m_n+=1900;
		if(m_y<10) m_y="0"+m_y;
		if(m_r<10) m_r="0"+m_r;
		if(m_h<10) m_h="0"+m_h;
		if(m_m<10) m_m="0"+m_m;
		if(m_s<10) m_s="0"+m_s;
		var m_theDate = ""+ m_n + "-" + m_y + "-" + m_r + " "+ m_h + ":" + m_m + ":" + m_s;
		$("#timelimit").html("阅读期限："+m_theDate);
	}
}

/***提交订单***/
function submitOrder()
{
	$("#docDownloadButton").attr("disabled","true");
	DOC88Window.setMessage($('#loadDiv').html());
	$.ajax({
		url: root_path+'doc.php',
		type: 'get',
		dataType: 'html',
		async: false,
		data: 'act=window_order&do_method='+currentDoType+'&txtPcode='+product_code+'&radioPriceid=1&method=ajax',
		success: function(data)
		{
			data = eval('(' + data + ')');
			if(data.result==1)
			{
				if(data.stateOrderChecking == 1){
					DOC88Window.setMessage($('#bldownloadinfoAlert').html());
					$("#blkorderinfodoc").html("您对此文档的订单, 正处于待审核状态, 请等待系统处理后, 再操作。");
					$("#blAlertPotions").html("&nbsp;");
					$("#docDownloadButton").hide();			
					document.getElementById("docCloseButton").value ="关闭";
					stateOrderChecking = 1;
				}else{
					reloadPageState(0);
				    checkDownload(0);					
				}

			}
			else
			{
				$("#blkorderinfodoc").html(data.message);
				$("#docDownloadButton").removeAttr("disabled");
			}
		}
	});
}

var m_checkingFile = false;
function m_checkFileExist(m_link,m_type)
{
	if(m_type==0&&m_checkingFile)
	{
	  return;
	}
	m_checkingFile = true;
 	$.ajax({
		url: "/doc.php?act=cse&pcode="+product_code,
		dataType:"html",
		async: false,            		            		          		
		success: function(msg){
		     if(msg=="1")
		     {
		         window.location.href = m_link;
				 $("#blkorderinfodoc").html("");
				 windowClose();
				 m_checkingFile = false;
		     }
		     else
		     {
		      window.setTimeout(function(){m_checkFileExist(m_link,1)},3000);
		     }
		}
	});
}
/****下载/阅读/打印文档****/
function checkDownload(downloadType){
	
	var downloadLabel = "如果未弹出文件保存对话框，请到“<a href='/portal/usr_order.php?act=bought' style='color:#1E35A5;' target='_blank'>会员中心</a>-><a href='/portal/usr_order.php?act=bought' style='color:#1E35A5;' target='_blank'>已下载文档</a>”页面下载。";
	//文档下载/阅读
	if(currentDoType==0)
	{
	
		var stateDLRemainTimes = $("#stateDLRemainTimes").val();
		if(stateDLRemainTimes=="")
			stateDLRemainTimes = 0;
		if(stateDLRemainTimes>0||ownPublish==1)
		{
			$.ajax({
				url: root_path+"member.php",
				type: 'get',
				dataType: 'html',
				data: 'act=ajax_checkdownload',
				async: false,
				success: function(data){
					$.ajax({
		            		url: "/doc.php?act=download&pcode="+product_code+"&dlrand="+data+"&dlcode=&t=1",
		            		dataType:"html",
		            		async: false,            		            		          		
		            		success: function(msg)
					{
						 msg = eval('(' + msg + ')');
						 if(msg.e==1)
						 {
							window.location.href = msg.link;
							windowClose();
						 }
						 else
						 {
						 	windowClose();
							DOC88Window.win({message: $('#loadDiv').html(),width:m_getDownWinWidth(),height:270,title:'文档下载',iframe:false});
							m_checkFileExist(msg.link,0);
						 }
						if(checkdl!=1){
							curDownloadTimes--;
						}
						return false;
		            		}
		            });
	
				}
			});
		}
		else
		{
			if(priceFree==1)
			{
				submitOrder();
			}
			else
			{
				DOC88Window.setIFrame("_blank");
				$("#form_productbuy").submit();
				return false;
			}
		}
		
	}

}
	
//收藏
function checkcollectproduct(){
	addToFolder(product_id,"view");
}

//关注好友
/**添加某个用户为好友****/
function addToFriend(member_id)
{
		$.ajax({
            url: '/portal/usr_friend.php',
            type: 'get',
            dataType: 'json',//
            data: 'act=ajax_addFriend&friend_id='+member_id,
            success: function(msg)
			{
				if(msg.stateLogined==0)
				{
					DOC88Window.win({message: '/js/home/window.html?act=window_login&actMethod=addFriend&',width:600,height:200,title:'添加好友',iframe:true});
					return;
				}
				if(msg.self)
				{
					showTipWin("添加失败，不能添加自己为好友!");
					return ;
				}
				if(msg.exist)
				{
					showTipWin("您已经成功添加"+nick_name+"为好友!");
					return;
				}
				showTipWin("您已经成功添加"+nick_name+"为好友!");
            }
        });
        return;
 }

//举报文档
function complaint()
{
	shortcutComplaint();
}
function showWXDeposit()
{
	$("#depositpopup").hide();
	$("#newpopup").show();
	document.getElementById('ym-header-text').innerHTML="微信扫码充值兑换积分";
}
function showDepositWin()
{
	document.getElementById('ym-header-text').innerHTML="使用其他方式充值";
	$("#newpopup").hide();
	$("#depositpopup").show();
}
/****举报登陆***/
function windowLoginForComplaint()
{
	DOC88Window.setIFrame('/js/home/window.html?act=window_login&actMethod=complaint&p_code='+product_code+'&logintitle='+encodeURIComponent('登录网站在线提交您的举报信息'));
}
//快捷举报文档
function shortcutComplaint()
{
	DOC88Window.win({message: '/info.php?t=1&act=shortcut_complaint&actMethod=complaint&p_code='+product_code,width:600,height:286,height:456,title:'举报文档',iframe:true});
}
//侵权举报
function complaintAlert()
{
	DOC88Window.resizeWin(600,200);
	DOC88Window.setMessage($('#complainInfo').html());
}
function setOrderChecking(m_orderCheck){
    stateOrderChecking = m_orderCheck;
}
function download_nologin_scan()
{
//	DOC88Window.win({message: ' ,width:600,height:270,title:'微信支付',iframe:true});
	DOC88Window.setIFrame('/doc.php?act=wxpay_iframe_show&money='+freeDLprice);
}
function app_scan_download(){
	document.getElementById('ym-header-text').innerHTML="安装APP下载此文档";
	DOC88Window.setIFrame('/doc.php?act=app_scan_download&money='+freeDLprice);
}
//微信阅读页面
function weixin_read(pcode){
	 DOC88Window.win({
         message: '/doc.php?act=readqrcode&p_code='+pcode,
         width: 320,
         height: 370,
         title: '微信扫码用手机阅读',
         iframe: true
     });
}
//微信阅读页面
function app_read(pcode){
	 DOC88Window.win({
         message: '/doc.php?act=qrcode&p_code='+pcode+'&from=1',
         width: 642,
         height: 370,
         title: 'APP扫码阅读',
         iframe: true
     });
}
function goto_renwu(){
	window.open("/renwu/find.php?f="+window.location.href);
	return false;
}
function goto_uploadfile(){
	window.open("/uc/index.php?act=upload");
	return false;
}