var log_preNum = 0;
var toolHeight = 45;//46减去一像素边框
var scrollFlag = true;
var ft = 0;
var ftup = 0;
var pageTime = [];
var pageWidth = 1130,
	playerWidth = 802;
var bigScreen = screen.width > 1440;
if (bigScreen && typeof endBigScreen != 'undefined') {
	var css = document.createElement('link');
	css.href=picture_image_path_v1+'/css/end/newEndFlatScreenBig.css?rand=20160519';
	css.rel="stylesheet";
	css.type="text/css";
	with(document)0[(getElementsByTagName('head')[0]||body).appendChild(css)];
	pageWidth = 1222;
	playerWidth = 894;
}
if(jQuery(".j_sidercontrol").length>0){
	var oRelaTop = jQuery(".j_sidercontrol").offset().top;
}
var relateDocH = 0;
function DocinReaderPlayer(){
	if(arguments.length==1){ 
		var arg = arguments[0];
	}
	this.config = {
		productId:0,
		allPage:0,
		previewNum:0,
		isBuy:1,
		baseUrl:"",
		flashUrl:picture_image_path_v1+"/players/PageViewer.swf?",
		initZoom:0,
		showStyle:1,
		contStyle:1,
		bookMark:0,
		isLogin:0,
		bookmarkid:-1,
		showAd:true,
		yjAdvStyle:'00',
		load_url:""
	};

	for(var p in arg){
		this.config[p] = arg[p];
	}
	this.iWidth = [792,pageWidth];
	this.iHeight = [];
	this.metas={0:{width:720,height:540},1:{width:720,height:540},2:{width:720,height:540}};//记录文档高度
	this.screenWidth = window.screen.width;
	this.btn_zoomIn = jQuery("#j_zoomin");
	this.btn_zoomOut = jQuery("#j_zoomout");
	this.btn_comment = jQuery(".page_comment");
	this.btn_upload = jQuery(".page_upload");
	this.btn_search = jQuery(".page_search");
	this.btn_bookmark = jQuery(".book_mark");
	this.btn_fullscreen = jQuery(".btn_fullscreen");
	this.preBtn = jQuery("#preview"),
	this.nextBtn = jQuery("#nextPage");
	this.menu = jQuery('.doc_readmod,.reader_menu');
	this.topBtn = jQuery(".top_like");
	this.hitBtn = jQuery(".step_on");
	this.zoomFlag = 0;
	this.curPage = 1;
	this.myScale = 1;
	this.flashSize = {width:0,height:0};
	this.gotoPageInput = jQuery(".page_cur");
	this.floderBtn = jQuery("#floderbtn");
	this.adTj = {'1':[923504,923505,923506,923507,923508],'2':[939345,939347,939348,939349,939350]};
	//this.init();//是图片格式的时候直接初始化
	if(this.config.docstatus == 0){
		this.pageScroll(jQuery);
		this.getMeta();//是flash格式的时候执行此方法
	}
	else{
		var screenH = jQuery(window).height(),
		pageOffsetTop = jQuery("#contentcontainer").offset().top;
		var pageNull = jQuery('<div id="pagenull" class="model panel" style="margin:0;"></div>').appendTo(jQuery("#contentcontainer"));
		jQuery("#pagenull").css({height:screenH-pageOffsetTop-50});
		jQuery("#floderbtn").hide();
		jQuery(".aside").css({marginTop:41});
		if(this.config.docstatus == 1){
			var ostatusP = jQuery('<p class="docStatusTips"><span class="txt">文档转换中</span></p>');
		}
		else if(this.config.docstatus == 2){
			var ostatusP = jQuery('<p class="docStatusTips"><span class="txt">文档审核中</span></p>');
		}
		else if(this.config.docstatus == 3){
			var ostatusP = jQuery('<p class="docStatusTips convertFailed"><span class="txt">该文档转化失败，<br/>请<a href="javascript:void(0);" onclick="downLoadNew('+this.config.productId+','+"1"+')">下载</a>后阅读。</span></p>');
		}
		ostatusP.appendTo(jQuery("#pagenull"));
		likeTooModControl(1);
	}
}
DocinReaderPlayer.prototype = {
	init:function(){
		var _this = this;
		_this.getMyScale();
		if(jQuery("#beforeAd").length>0){
			//docin_adload('133','qiantiepian_adv');
			jQuery("#beforeAd").css({width:playerWidth}).show();
			if(!window.XMLHttpRequest){
				var oHeight = jQuery("#contentcontainer").height();
				jQuery("#beforeAd").css({height:oHeight});
			}
		}
		_this.createDiv();
		_this.setBeforeAd();//前贴片
		if(typeof(qwpart)!="undefined" && qwpart != 0){
			
			_this.gotoPages(qwpart,1);
		}
		if(typeof(showPageNum)!="undefined"){
			_this.gotoPages(showPageNum,1);
		}
		if(typeof(j_qtp)!= "undefined"){
			if(j_qtp == 1){
				_this.countHide(8,'endNum','beforeAd');//前贴片倒计时关闭
			}else{
				_this.countHide(15,'endNum','beforeAd');//前贴片倒计时关闭
			}
		}
		
		if(typeof(isCartOrisCode)=="undefined"&&(_this.config.isBFD == false||_this.config.isBFD == undefined)){
			_this.loadRelative();
		}
		if(showComment == "message"){
			_this.commentHandel();
		}
		_this.pageButton(_this.curPage);
		_this.zoomOut();
		_this.zoomIn();
		_this.fullscreen();
		_this.oSiderTop = jQuery(".aside").offset().top;
		_this.setPage(window);
		_this.initZoom();
		_this.prePage();
		_this.nextPage();
		_this.addBookMark();
		_this.jumpToPage();
		_this.readerMenu();
		_this.tabChangeHand();
		_this.initComment();
		_this.handMove();
		_this.hideSelected();
		if(typeof(isBuilding)!="undefined"){
			_this.setFloderBtn();//建筑新增代码
		}
		if(_this.config.showStyle == 2){
			jQuery(".doc_reader_mod").addClass('style_pic');
			jQuery("#maskLeft").show();
			jQuery("#maskRight").show();
			_this.cursorPage();
			if(jQuery("#j_hand").length>0&&jQuery("#j_select").length>0){
				jQuery("#j_hand").removeClass('drag_hand,btn_cur').addClass('drag_hand_no');
				jQuery("#j_select").removeClass('select_hand,btn_cur').addClass('select_hand_no');
				if(jQuery('#j_comdoc').length > 0){
					jQuery("#j_hand").hide(0);
					jQuery("#j_select").show(0);
				}
			}
			if(typeof fromWebSearch !="undefined"){
				fromWebSearch();
			}
		}
		jQuery(".page_num").html('/'+_this.config.allPage);
		//如果书签存在的情况
		if(_this.config.bookMark>0 && qwpart == 0){
			_this.curPage = _this.config.bookMark;
			_this.gotoPage(_this.curPage,1);
			_this.btn_bookmark.addClass('book_mark_cur');
			jQuery("#bookmarktips p").html('你上次阅读到此页,点击这里返回第1页');
			jQuery("#bookmarktips").fadeIn().bind('click',function(){
				_this.gotoPage(1,1);
			});
			_this.setBookMark();
		}
		jQuery(document).bind('keydown',function(ev){
			if(ev.keyCode == 27&&_this.zoomFlag>0){
				_this.zoomInStyle(0);
			}
		});
		_this.floderBtn.bind('click',function(){
			if(jQuery(this).hasClass('btn_close')){
				_this.zoomInStyle(0);
				inpmv(5108);
			}
			else{
				jQuery(this).addClass('btn_close');
				_this.zoomInStyle(1);
				inpmv(5107);
			}
			_this.setBeforeAd();
		});
		//分享
		_this.btn_upload.bind('click',function(){
			_this.openShare(_this.config.productId);
		});
		if(jQuery(".new_share_more,.slide-more").length>0){
			jQuery(".new_share_more,.slide-more").bind("click",function(){
				_this.openShare(_this.config.productId);
			});
		}
		//付费阅读购买
		var oPayBtn = jQuery("#payForRead"),
			oGotoReadBtn = jQuery("#gotoRead");
		if(oGotoReadBtn.length>0){
			oGotoReadBtn.bind('click',function(){
				purchaseProduct(js_pid,tmp_userId,'buy');
			});
		}
		if(oPayBtn.length>0){
			oPayBtn.bind('click',function(){
				//if(_this.config.isLogin == 0){
					//showlogin();
					//return false;
				//}
				//purchasePayProduct(_this.config.productId,tmp_userId);//余额不足，弹出充值层
				clickPayRead(_this.config.productId,tmp_userId);
			});
		}
		_this.gotoPageInput.focusin(function(){
			jQuery(this).select();
		});
		jQuery("#bookmarktips .close").bind('click',function(event){
			jQuery("#bookmarktips").fadeOut();
			event.stopPropagation();
		});
	},
	tabInit:function(){
		var _this = this;
		_this.createDiv();
		if(_this.config.showStyle == 1){
			_this.setZoomFlashSize(_this.zoomFlag);
			if(_this.zoomFlag == 0){
				jQuery("#contentcontainer").css('paddingTop',0);
			}
			jQuery(".reader_container").css('height','auto');
			if(jQuery(".recommed_shadow").length>0&&jQuery(".recommedMod").length>0){
				jQuery(".recommed_shadow,.recommedMod").hide();
			}
		}
		_this.setPage(window);
		if(_this.config.showStyle == 2){
			_this.cursorPage();
			if(_this.zoomFlag == 0||_this.zoomFlag == 1){
				if(typeof fromWebSearch !="undefined"){
					fromWebSearch();
				}
			}
		}
		if(_this.zoomFlag == 1||_this.zoomFlag == 2||_this.zoomFlag ==3){
			setToolBarFix(2);
			siderFixedMove(2);
			jQuery("#contentcontainer").css('paddingTop',toolHeight);
		}
		jQuery(document).bind('keydown',function(ev){
			if(ev.keyCode == 27&&_this.zoomFlag>0){
				_this.zoomInStyle(0);
			}
		});
		log_preNum = 0;
		_this.gotoPage(_this.curPage,1);
		if(_this.config.showStyle == 1){
			jQuery(".scrollLoading").scrollLoading();
		}
		if((_this.zoomFlag == 2&&_this.config.showStyle == 2)||(_this.zoomFlag == 3&&_this.config.showStyle == 2)){
			if(jQuery(".backToTop").length>0){
				jQuery(".backToTop").hide();
			}
		}
		else{
			setBarPosition();
		}
		_this.zoomInStyle(_this.zoomFlag);
		_this.hideSelected();
	},
	commentHandel:function(){
		var _this = this;
		jQuery.ajax({
			url:'/jsp_cn/productEnd/load/doc_end_load_comment.jsp',
			data:'isLogin='+_this.config.isLogin,
			context: document.body,
			success:function(data){
				var oHtml = data;
				jQuery(this).append(oHtml);
				var cH = jQuery(window).height();
				var oCommentHeight = cH - cH*0.2 -142;
				jQuery("#showMsgContent").height(oCommentHeight);
				dialogBoxShadow();
				setObjCenter('j_comment');
				showMessageContent();
				var oTextArea = jQuery("#message");
				oYzm = jQuery("#checkout");
				if(oTextArea.length == 0){return;}
				oTextArea.bind('keydown',function(event){
					if(event.ctrlKey&&event.keyCode == 13){
						insertUserMessage(1);
					}
				});
				if(oYzm.length>0){
					oYzm.bind('keydown',function(event){
						if(event.ctrlKey&&event.keyCode == 13){
							insertUserMessage(1);
						}
					});
				}
				oTextArea.bind('focus',function(){
					showCheckCode();
				});
				jQuery('textarea[placeholder]').placeholder();
				oTextArea.focus();
			}
			});
	},
	initComment:function(){
		var _this = this;
		jQuery(".page_comment,#showComm").bind('click',function(){
			if(jQuery("#jCommentDiv").length>0){
				var iCommentTop = jQuery("#jCommentDiv").offset().top;
				jQuery(window).scrollTop(iCommentTop);
			}
			else{
				_this.commentHandel();
			}
		});
	},
	openShare:function(pid){
		var popTmp = jQuery("#docinPopBox1");
			if(popTmp.length>0){
				dialogBoxShadow();
				setObjCenter('docinPopBox1');
				docinCopyData('docUrl','textUrl');
				docinCopyData('flashUrl','flUrl');
				docinCopyData('embedTxt','endShareCode');
			}
			/**shareEvent(pid,13);
			//add at 2013-04-25 分享弹窗请求
			jQuery.ajax({
				type:"POST",
				url:"/jsp_cn/productEnd/share_new_v1.jsp",
				context: document.body,
				data:"productid="+pid,
				async: false,
				success:function(re){
					if(re){
						jQuery(this).append(re);
						dialogBoxShadow();
						setObjCenter('docinPopBox1');
						docinCopyData('docUrl','textUrl');
						docinCopyData('flashUrl','flUrl');
						docinCopyData('embedTxt','endShareCode');
					}
				}
			});**/
	},
	clearFlash:function(pageno){
		var _this = this;
		_this.curPage = pageno;
		var aPages = jQuery("#contentcontainer .panel");
		jQuery.each(aPages,function(i){
			var tempI = i+1;
			if(tempI == pageno){return true;}
			if(tempI == pageno+1){return true;}
			if(tempI == pageno-1){return true;}
			jQuery(this).find("#flash_"+(i+1)).replaceWith('<div id="flash_'+(i+1)+'"></div>');
		});
	},
	addFlash:function(pageno){
		var _this = this;
		for(var p = pageno-1;p<=pageno+1;p++){
			if(p != 0&&jQuery("div#flash_"+p).length>0){
				var  flashvars={productId:readerConfig.productId,pagenum:p-1,loadurl:_this.config.load_url},params = {wmode:'transparent',allowscriptaccess:'always',hasPriority:'true'},attributes={};
				swfobject.embedSWF(_this.config.flashUrl, "flash_"+p, '100%', '100%', "9.0.0",picture_image_path_v1+'/js/expressInstall.swf',flashvars,params,attributes);
			}
		}
	},
	setLoading:function(){
		var iPage1Top = Math.ceil(jQuery("#contentcontainer").offset().top);
		var iViewHeight = jQuery(window).height();
		var loadingTop = (iViewHeight - iPage1Top)/2+"px";
		jQuery('.reader_container .panel_inner').eq(0).css("backgroundPosition","center "+loadingTop);
	},
	createDiv:function(){
		var _this = this;
		function createPageDiv(f){

			if(jQuery('#j_comdoc').length > 0){

				var nComPages = comdocsConf.zhtotal;

				var sAddToComDoc = '<div class="addcomdocs addcomdocs-on" title="新功能：把任意文档的任何一页加入组合成一个新文档"><span class="comdoc-ico"></span><p>加入我的组合</p></div>',
					aPages = comdocsConf.zhPages.split(",");

				if(nComPages > 0 && jQuery('#comdoc_num').length!=1){
					jQuery('.comdoc_btn_wrap').html(jQuery('.comdoc_btn_wrap').html()+'<em class="comdoc_num" id="comdoc_num">'+nComPages+'</em>');
				}

				for(var j=0; j<aPages.length; j++){
					if(i == aPages[j]){
						sAddToComDoc ='<div class="addcomdocs addcomdocs-off"><span class="comdoc-ico"></span><p>已加入</p></div>';
					}
				}

				if(f == 1){
					var pageDiv = jQuery('<div id="page_'+i+'" class="model panel">'+ sAddToComDoc +'<div class="panel_inner scrollLoading" data-url="'+_this.config.flashUrl+'"><div id="flash_'+i+'"></div></div></div>').appendTo(jQuery("#contentcontainer"));
						jQuery("#page_"+i+" .panel_inner").css({width:_this.setFlashSize(0,i)[0],height:_this.setFlashSize(0,i)[1]});
				}
				else if(f == 2){
					var pageDiv = jQuery('<div style="width:0px;height:0px;" id="page_'+i+'" class="model panel">'+ sAddToComDoc +'<div class="panel_inner"><div id="flash_'+i+'"></div></div></div>').appendTo(jQuery("#contentcontainer"));
				}

				jQuery(".addcomdocs-on").bind('mouseover',function(event){
					event.stopPropagation();
					jQuery(this).addClass('addcomdocs-hover');
				}).bind('mouseout',function(event){
					event.stopPropagation();
					jQuery(this).removeClass('addcomdocs-hover');
				});

			}else{

				if(f == 1){
					var pageDiv = jQuery('<div id="page_'+i+'" class="model panel"><div class="panel_inner scrollLoading" data-url="'+_this.config.flashUrl+'"><div id="flash_'+i+'"></div></div></div>').appendTo(jQuery("#contentcontainer"));
						jQuery("#page_"+i+" .panel_inner").css({width:_this.setFlashSize(0,i)[0],height:_this.setFlashSize(0,i)[1]});
				}
				else if(f == 2){
					var pageDiv = jQuery('<div style="width:0px;height:0px;" id="page_'+i+'" class="model panel"><div class="panel_inner"><div id="flash_'+i+'"></div></div></div>').appendTo(jQuery("#contentcontainer"));
				}
			}
		}
		if(_this.config.showStyle == 1){//1：竖版 2：横版
			if(_this.config.isBuy == 1){//已付费
				jQuery("#contentcontainer").html("");
				var createNum = _this.config.allPage;
				for(var i = 1;i<=createNum;i++){
					createPageDiv(1);
					if(i == 1){_this.setLoading();}
				}
				jQuery('<div id="moveHandel"></div>').appendTo(jQuery("#contentcontainer"));
			}
			else if(_this.config.isBuy == 0){//未付费，只加载预览页数
				jQuery("#contentcontainer").html("");
				var createNum = _this.config.previewNum;
				for(var i = 1;i<=createNum;i++){
					createPageDiv(1);
				}
				jQuery('<div id="moveHandel"></div>').appendTo(jQuery("#contentcontainer"));
			}
			else if(_this.config.isBuy == 2){//付费或下载完 加载付费页数
				for(var i =_this.config.previewNum+1;i<=_this.config.allPage;i++){
					createPageDiv(1);
					if(i == 1){_this.setLoading();}
				}
				_this.config.isBuy = 1;
			}
			if(jQuery(".preview_end").length>0){
				jQuery(".preview_end").show(); 
			}

			if(jQuery("#j_isend").length>0){
				jQuery("#j_isend").show();
			}
			if(_this.config.showAd == true){
				_this.insertAd(2);//A样式
			}
			jQuery(".scrollLoading").scrollLoading();
			likeTooModControl(1);
		}
		else if(_this.config.showStyle == 2){
			if(jQuery("#j_isend").length>0){
				jQuery("#j_isend").hide(); 
			}
			if(_this.config.isBuy == 1){
				jQuery("#contentcontainer").html("");
				var createNum = _this.config.allPage;
				for(var i = 1;i<=createNum;i++){
					createPageDiv(2);
				}
				jQuery("#page_1").removeAttr('style');
			}
			else if(_this.config.isBuy == 0){
				jQuery("#contentcontainer").html("");
				var createNum = _this.config.previewNum;
				for(var i = 1;i<=createNum;i++){
					createPageDiv(2)
				}
				jQuery("#page_1").removeAttr('style');
			}
			else if(_this.config.isBuy == 2){
				for(var i = _this.config.previewNum+1;i<=_this.config.allPage;i++){
					createPageDiv(2)
				}
				jQuery("#page_"+_this.curPage).removeAttr('style');
				_this.config.isBuy = 1;
			}
			_this.setReaderHeight();
			_this.addFlash(_this.curPage);
			_this.clearFlash(_this.curPage);
			likeTooModControl(1);
		}
	},
	setFlashSize:function(iZoom,pageno){
		var _this = this;
		var iHeight = parseInt(_this.iWidth[iZoom]/_this.myScale);
		_this.iHeight[iZoom] = iHeight;
		return [_this.iWidth[iZoom],iHeight];
	},
	setZoomFlashSize:function(iZoom){
		var _this = this;
		var aFlash = jQuery("#contentcontainer .panel_inner");

		if(_this.screenWidth<=1280){
			_this.iWidth[2] = 1240;	
			_this.iWidth[3] = 1695;	
		}
		else if(_this.screenWidth>1280){
			_this.iWidth[2] = _this.screenWidth-40;
			_this.iWidth[3] = _this.screenWidth*1.5;
		}
		else if(_this.screenWidth>2560){
			_this.iWidth[2] = 2520;
			_this.iWidth[3] = 3840;
		}
		jQuery.each(aFlash,function(i){
			var zoomHeight = parseInt(_this.iWidth[iZoom]/_this.myScale);
			_this.iHeight[iZoom] = zoomHeight;
			jQuery(this).css({width:_this.iWidth[iZoom],height:zoomHeight});
		});
		if(iZoom == 3){
			var sl = (_this.iWidth[3]-jQuery(window).width())/2;
			jQuery(window).scrollLeft(sl);
		}
	},
	getMyScale:function(){
		var _this = this;
		//var pageIndex = parseInt(pageno/50);
		var pageIndex = 0;
		_this.myScale =_this.getScale(_this.metas[pageIndex].width,_this.metas[pageIndex].height);
	},
	getScale:function(w,h){
		var _this = this;
		var iScale = (w/h).toFixed(4);
		return iScale;
	},
	setReaderHeight:function(){
		var _this = this;
		var st = jQuery(window).scrollTop();
		var oReaderTop = Math.ceil(jQuery(".reader_container").offset().top),
			oScreenHeight = jQuery(window).height(),
			oToolHeight = jQuery(".docin_reader_tools").outerHeight();
			if(oReaderTop>206){oReaderTop = 206;}
			if(_this.zoomFlag == 0){
				var cH = oScreenHeight-oReaderTop;
				var docHeight = cH-2;
			}
			if(_this.zoomFlag>0){
				var cH = oScreenHeight;
				var docHeight = cH - oToolHeight-2;
			}
			if(docHeight<390){
				docHeight = 420;
			}
			var cW = jQuery("#contentcontainer").width();
			var tempWidth =Math.ceil(docHeight*_this.myScale);//如果适高
			var tempHeight =Math.ceil(cW/_this.myScale);//如果适宽
			if(tempWidth>cW){//按照宽来
				if(_this.zoomFlag == 0){
					jQuery(".reader_container").css('height',tempHeight);
					jQuery(".panel_inner").css({width:cW,height:tempHeight});
				}
				else if(_this.zoomFlag == 1){
					jQuery(".reader_container").css('height',tempHeight);
					jQuery(".panel_inner").css({width:cW,height:tempHeight});
					jQuery("#contentcontainer").css('paddingTop',oToolHeight);
				}
				else{
					jQuery(".reader_container").css('height',cH-2);
					jQuery(".panel_inner").css({width:cW,height:tempHeight});
					var mart = Math.floor(cH - tempHeight)/2;
					jQuery("#contentcontainer").css('paddingTop',mart);
				}
			}
			else{//按照高来
				jQuery(".panel_inner").css({width:tempWidth,height:docHeight});
				if(_this.zoomFlag == 0){
					jQuery(".reader_container").css('height',docHeight);
					jQuery("#contentcontainer").css('paddingTop',0);
				}
				else if(_this.zoomFlag>0){
					jQuery(".reader_container").css('height',cH-2);
					jQuery("#contentcontainer").css('paddingTop',oToolHeight);
				}
			}
	},
	cursorPage:function(){
		//判断鼠标的位置
		var _this = this;
		var oMaskLeft = jQuery("#maskLeft"),
			oMaskRight = jQuery("#maskRight");

		oMaskLeft.bind('mouseover',function(event){
			oMaskLeft.css({cursor:'url('+picture_image_path_v1+'/images_cn/news/html_reader/arr_left.cur),auto'});
		});
		oMaskRight.bind('mouseover',function(event){
			oMaskRight.css({cursor:'url('+picture_image_path_v1+'/images_cn/news/html_reader/arr_right.cur),auto'});
		});
		oMaskLeft.unbind('click');
		oMaskLeft.bind('click',function(){
			_this.preBtn.triggerHandler("click");
		});
		oMaskRight.unbind('click');
		oMaskRight.bind('click',function(){
			_this.nextBtn.triggerHandler("click");
		});
		jQuery(document).unbind("keydown");
		jQuery(document).bind("keydown",function(event){
			if(_this.config.showStyle == 2){
				_this.tabKeyPage(event);
			}
		});
	},
	tabKeyPage:function(event){
		var _this = this;
		var oKey = event.keyCode; 
		if(oKey == 37){
			_this.preBtn.triggerHandler("click");
			inpmv(5464);
			return;
		}
		if(oKey == 39){
			_this.nextBtn.triggerHandler("click");
			inpmv(5466);
			return;
		}
	},
	getMeta:function(){
		var _this = this;
		// var aJaxNum = Math.ceil(_this.config.allPage/50);
		// aJaxNum = 1;
		
//		if(_this.config.productId%4 == 1){
//			metaBaseUrl = metaBaseUrl.replace("page.douding.cn","221.122.117.68");
//		}
//		if(_this.config.productId%4 == 2){
//			metaBaseUrl = metaBaseUrl.replace("page.douding.cn","221.122.117.69");
//		}
//		if(_this.config.productId%4 == 3){
//			metaBaseUrl = metaBaseUrl.replace("page.douding.cn","221.122.117.70");
//		}
//		if(_this.config.productId%4 == 0){
//			metaBaseUrl = metaBaseUrl.replace("page.douding.cn","221.122.117.72");
//		}
		var metaBaseUrl = 'http://page.douding.cn/docinfile2/meta_'+_this.config.productId+'_';
		jQuery.ajax({
				type:'get',
				url:metaBaseUrl+'0.docin',
				dataType: 'jsonp',
				success:function(data){
					_this.metas[0] = data;
					_this.init();

				},
				error:function(){
					//倒霉，没有获取到
				}
			});
		// for(var i = 0;i<aJaxNum;i++){
			// jQuery.ajax({
			// 	type:'get',
			// 	url:metaBaseUrl+i+'.docin',
			// 	dataType: 'jsonp',
			// 	success:function(data){
			// 		_this.metas[0] = data;
			// 		_this.init();

			// 	},
			// 	error:function(){
			// 		//倒霉，没有获取到
			// 	}
			// });
		// }
	},
	initZoom:function(){
		var _this = this;
		if(this.config.initZoom ==1){
			_this.zoomInStyle(1);
		}
		else if(this.config.initZoom ==2){
			_this.zoomInStyle(2);
		}
		else if(this.config.initZoom ==0){
			_this.zoomInStyle(0);
		}
	},
	fullscreen:function(){
		var _this = this;
		_this.btn_fullscreen.bind('click',function(){
			if(jQuery(this).hasClass('q')){//正在全屏,点击退出
				_this.zoomInStyle(0);
				inpmv(5118);
			}
			else{
				_this.zoomInStyle(2);
				_this.floderBtn.hide();
				inpmv(5070);
			}
			
		});
	},
	escapeFullScreen:function(){
		var _this = this;
		_this.zoomFlag = 0;
		jQuery(".page_body").removeClass('full_2');
		jQuery(".page_body").removeClass('full_3');//退出大全屏
		jQuery(".main").removeClass('full_1');
		_this.btn_zoomOut.addClass('zoom_out_no').removeClass('zoom_out');
		_this.controlSomeDiv(1);
		_this.btn_fullscreen.removeClass('q');
		_this.btn_fullscreen.html('全屏');
		_this.btn_zoomIn.removeClass('zoom_in_no').addClass('zoom_in');
		if(_this.config.showStyle == 1){
			_this.setZoomFlashSize(0);
		}
		if(_this.config.showStyle == 2){
			_this.setReaderHeight();
		}
		_this.gotoPage(_this.curPage);
		if(jQuery(".w100").length>0){
			jQuery(".w100").show();
		}
		_this.floderBtn.show().removeClass('btn_close');
	},
	zoomOut:function(){
		var _this = this;
		if(_this.btn_zoomOut.length>0){
			_this.btn_zoomOut.bind('click',function(){
				_this.btn_fullscreen.html('全屏');
				if(_this.zoomFlag == 3){
					jQuery(".page_body").removeClass('full_3');
					_this.zoomInStyle(2);
				}
				else if(_this.zoomFlag == 2){
					_this.zoomInStyle(1);
				}
				else if(_this.zoomFlag == 1){
					_this.floderBtn.show().removeClass('btn_close');
					_this.zoomInStyle(0);
				}
				else if(_this.zoomFlag == 0){
					return false;
				}
								
			});
		}
	},
	zoomIn:function(){
		var _this = this;
		if(_this.btn_zoomIn.length>0){
			_this.btn_zoomIn.bind('click',function(){
				if(_this.zoomFlag == 0){
					_this.zoomInStyle(1);
				}
				else if(_this.zoomFlag == 1){
					_this.zoomInStyle(2);
					_this.floderBtn.hide();
				}
				else if(_this.zoomFlag == 2){
					_this.zoomInStyle(3);
					_this.floderBtn.hide();
				}
			});
		}
	},
	zoomInStyle:function(f){
		var _this = this;
		if(_this.zoomFlag == 0&&jQuery(window).scrollTop()<=_this.oSiderTop){
			_this.temoScale = (jQuery(window).scrollTop()+(jQuery(window).height()/3)+166)/jQuery(document).height();
		}else{
			_this.temoScale = (jQuery(window).scrollTop()+(jQuery(window).height()/3))/jQuery(document).height();
		}
		if(f == 0){
			jQuery(".page_wrap").css({marginTop:50});
			if(!window.XMLHttpRequest){jQuery(".page_wrap").css({marginTop:0});}
			_this.escapeFullScreen();
			if(jQuery(".w100").length>0){
				jQuery(".w100").show();
			}
			setToolBarFix(1);
			siderFixedMove(1);
			if(_this.config.showStyle == 2){
				jQuery(".main").css({width:playerWidth});
				_this.setReaderHeight();
			}
			likeTooModControl(1);
			_this.btn_fullscreen.html('全屏').removeClass('q').data("label",'全屏');
			if(_this.config.showStyle == 1){
				jQuery(".main").css({width:playerWidth});
				jQuery("#contentcontainer").css({paddingTop:0});
				if(typeof(isBuilding)!="undefined"){
				_this.relativeControl();//建筑终极页新增代码
				}
			}
			if(typeof(isBuilding)!="undefined"){
				_this.setFloderBtn();//建筑新增代码
			}
			_this.gotoPage(_this.curPage);
			setBarPosition();
			controlLeftBottomAdv(1);
			return;
		}
		jQuery(".page_wrap").css({marginTop:0});
		_this.controlSomeDiv(0);//隐藏不需要的div
		if(f == 1){
			_this.zoomFlag = 1;
			jQuery(".main").addClass('full_1');
			if(_this.config.showStyle == 1){
				jQuery(".full_1").css({width:_this.iWidth[1]});
			}
			_this.btn_zoomOut.removeClass('zoom_out_no').addClass('zoom_out');
			_this.btn_zoomIn.removeClass('zoom_in_no').addClass('zoom_in');
			_this.floderBtn.addClass('btn_close').show();
			_this.btn_fullscreen.html('全屏').removeClass('q').data("label",'全屏');
			jQuery(".page_body").removeClass('full_2');
			jQuery(".page_body").removeClass('full_3');
			if(_this.config.showStyle == 2){
				setBarPosition();
				jQuery(".main").css({width:'100%'});
			}
			likeTooModControl(1);
			controlLeftBottomAdv(2);
		}
		if(f == 2){
			_this.zoomFlag = 2;
			jQuery(".main").addClass('full_1');
			jQuery(".page_body").addClass('full_2');
			if(_this.config.showStyle == 1){
				jQuery(".full_1").css({width:_this.iWidth[2]});
			}
			if(_this.config.showStyle == 2){
				jQuery(".main").css({width:'100%'});
			}
			_this.btn_zoomIn.removeClass('zoom_in_no').addClass('zoom_in');
			_this.btn_zoomOut.removeClass('zoom_out_no').addClass('zoom_out');
			_this.floderBtn.hide();
			_this.btn_fullscreen.html('取消全屏').addClass('q').data("label",'取消全屏');;
			likeTooModControl(2);
		}
		if(f == 3){
			_this.zoomFlag = 3;
			jQuery(".main").addClass('full_1');
			jQuery(".page_body").addClass('full_3');
			if(_this.config.showStyle == 1){
				jQuery(".full_1").css({width:_this.iWidth[3]});
			}
			if(_this.config.showStyle == 2){
				jQuery(".main").css({width:'100%'});
			}
			_this.floderBtn.hide();
			_this.btn_zoomIn.removeClass('zoom_in').addClass('zoom_in_no');
			_this.btn_zoomOut.removeClass('zoom_out_no').addClass('zoom_out');
			likeTooModControl(2);
		}
		if(_this.config.showStyle == 2){
			_this.setReaderHeight();
			if(f == 2||f == 3){
				if(jQuery(".backToTop").length>0){
					jQuery(".backToTop").hide();
				}
			}
		}
		else{
			setBarPosition();
		}
		jQuery("#contentcontainer").css('paddingTop',toolHeight);
		if(_this.config.showStyle == 1){
			_this.setZoomFlashSize(f);
		}
		setToolBarFix(2);
		siderFixedMove(2);
		_this.gotoPage(_this.curPage);
		if(jQuery(".w100").length>0){
			jQuery(".w100").hide();
		}
		if(typeof(isBuilding)!="undefined"){
			_this.setFloderBtn();//建筑新增代码
		}
	},
	controlSomeDiv:function(f){
		var _this = this;
		var aDiv = ['.user_doc_mod','.downloadArea2','.page_crubms','.doc_header_mod','.relative_doc_mod','.recent_cart_mod','.recent_doc_mod','.doc_hd_mini','.today_free','.sider_guanggao','.sider_guanggao_2','.col_box','.j_sidercontrol_build','.doc_hd','.doc_hds'];
		if(f == 0){
			jQuery.each(aDiv,function(i){
				if(jQuery(aDiv[i]).length>0){
					jQuery(aDiv[i]).hide();
				}
			})
		}
		if(f == 1){
			jQuery.each(aDiv,function(i){
				if(jQuery(aDiv[i]).length>0){
					jQuery(aDiv[i]).show();
				}
			})
		}
	},
	prePage:function(){
		var _this = this;
		_this.preBtn.bind('click',function(){
			if(_this.config.showStyle == 2){
				if(_this.flagPage == _this.config.allPage+1){
					_this.curPage = _this.flagPage;
					_this.flagPage = 0;
					if(jQuery(".recommed_shadow").length>0){
						jQuery(".recommed_shadow").hide();
					}
					if(jQuery(".recommedMod").length>0){
						jQuery(".recommedMod").hide();
					}
				}
			}
			_this.curPage --;
			if(_this.curPage<1){
				_this.curPage = 1;
				return;
			}
			_this.gotoPage(_this.curPage,2);
			_this.pageButton(_this.curPage);

		});
	},
	nextPage:function(){
		var _this = this;
		_this.nextBtn.bind('click',function(){
			_this.curPage ++;
			if(_this.curPage>_this.config.allPage){
				_this.curPage = _this.config.allPage;
				if(_this.config.showStyle == 2){
					_this.flagPage = _this.config.allPage+1;
					if(jQuery('.recommedMod').length>0){
						jQuery(".recommed_shadow").show();
						jQuery(".recommedMod").show();
						if(!window.XMLHttpRequest){
							jQuery(".recommed_shadow").height(jQuery("#contentcontainer").height());
						}
					}
					else{
						jQuery.ajax({
							url:'/app/p/end/ajax/getLastPageRecommends.do',
							data:'productId='+_this.config.productId,
							context:jQuery(".doc_reader_mod"),
							success:function(data){
								if(jQuery('.recommedMod').length>0&&jQuery(".recommed_shadow").length>0){
									return;
								}
								var oHtml = data;
								var oShadow = jQuery('<div class="recommed_shadow"></div>').appendTo(jQuery(this));
								jQuery(this).append(oHtml);
								jQuery("#jReview").bind("click",function(){
									_this.gotoPage(1);
									_this.pageButton(1);
									_this.curPage = 1;
									_this.flagPage = 0;
									jQuery(".recommed_shadow").hide();
									jQuery(".recommedMod").hide();
								});
								jQuery("#jCloseRecomm").bind("click",function(){
									_this.preBtn.click();
								});
								if(!window.XMLHttpRequest){
									jQuery(".recommed_shadow").height(jQuery("#contentcontainer").height());
								}
							}
						});
					}
				}
				return;}
			_this.gotoPage(_this.curPage,2);
			_this.pageButton(_this.curPage);
		});
	},
	gotoPages:function(pageno,k){
		var _this = this;
		if(_this.config.showStyle == 1){
			if(_this.config.isBuy == 0){
				if(pageno>_this.config.previewNum){
					_this.gotoPageInput.val(_this.config.previewNum);
					jQuery(window).scrollTop(jQuery(document.body).height());
					return false;
				}
			}
			if(jQuery("#page_"+pageno).length>0){
					//var preTop = jQuery("#page_"+(pageno-1)).offset().top;
					//var curObjTop = preTop+_this.iHeight[_this.zoomFlag]-54;
				if(pageno == 1){
					var preTop = jQuery("#page_"+pageno).offset().top;
					var curObjTop = 0;
				}
				else if(pageno == 2){
					if(jQuery(window).scrollTop()>_this.oSiderTop){
						var curObjTop = jQuery("#page_"+pageno).offset().top-44;
					}
					else{
						var curObjTop = jQuery("#page_"+pageno).offset().top-37-44;
					}
				}
				else{
					var preTop = jQuery("#page_"+pageno).offset().top-37-44;
					var curObjTop = preTop;
				}
					
			}
			if(k && k == 1){
				jQuery(window).scrollTop(curObjTop);
			}
			else if(k && k ==2){
				jQuery("html,body").stop().animate({ scrollTop:curObjTop}, 200);
			}
			else{
				if(_this.zoomFlag == 0&&jQuery(window).scrollTop()<=_this.oSiderTop){
					var ttt = _this.temoScale*jQuery(document).height()-jQuery(window).height()/3-166;
				}
				else{
					var ttt = _this.temoScale*jQuery(document).height()-jQuery(window).height()/3;
				}
				jQuery(window).scrollTop(ttt);
			}
		}
		if(_this.config.showStyle == 2){
			_this.curPage = pageno;
			if(_this.config.isBuy == 0){
				if(pageno>_this.config.previewNum){
					_this.gotoPageInput.val(_this.config.previewNum+1);
					_this.curPage = _this.config.previewNum+1;
					if(jQuery(".preview_end").length>0){
						jQuery(".model").css({width:0,height:0});
						jQuery(".preview_end").show();
					}
				}
				else{
					if(jQuery(".preview_end").length>0){
						jQuery(".preview_end").hide();
					}
					_this.gotoPageInput.val(_this.curPage);
					_this.addFlash(pageno);
					_this.clearFlash(pageno);
					jQuery(".model").css({width:0,height:0});
					jQuery("#page_"+pageno).removeAttr('style');
					_this.bookMarkShow(pageno);
					if(!window.XMLHttpRequest){
						var w = jQuery("#page_"+pageno).find(".panel_inner").width();
						var h = jQuery("#page_"+pageno).find(".panel_inner").height();
						jQuery("#page_"+pageno).css({width:w,height:h});
					}
				}
			}
			else{
				_this.gotoPageInput.val(_this.curPage);
				_this.addFlash(pageno);
				_this.clearFlash(pageno);
				jQuery(".model").css({width:0,height:0});
				jQuery("#page_"+pageno).removeAttr('style');
				_this.bookMarkShow(pageno);
				if(!window.XMLHttpRequest){
						var w = jQuery("#page_"+pageno).find(".panel_inner").width();
						var h = jQuery("#page_"+pageno).find(".panel_inner").height();
						jQuery("#page_"+pageno).css({width:w,height:h});
					}
			}
		}	
	},
	gotoPage:function(pageno,k){
		var _this = this;
		if(_this.config.showStyle == 1){
			if(_this.config.isBuy == 0){
				if(pageno>_this.config.previewNum){
					_this.gotoPageInput.val(_this.config.previewNum);
					jQuery(window).scrollTop(jQuery(document.body).height());
					return false;
				}
			}
			if(jQuery("#page_"+pageno).length>0){
					//var preTop = jQuery("#page_"+(pageno-1)).offset().top;
					//var curObjTop = preTop+_this.iHeight[_this.zoomFlag]-54;
				if(pageno == 1){
					var preTop = jQuery("#page_"+pageno).offset().top;
					var curObjTop = 0;
				}
				else if(pageno == 2){
					if(jQuery(window).scrollTop()>_this.oSiderTop){
						var curObjTop = jQuery("#page_"+pageno).offset().top-44;
					}
					else{
						var curObjTop = jQuery("#page_"+pageno).offset().top-37-44;
					}
				}
				else{
					var preTop = jQuery("#page_"+pageno).offset().top-44;
					var curObjTop = preTop;
				}
					
			}
			if(k && k == 1){
				jQuery(window).scrollTop(curObjTop);
			}
			else if(k && k ==2){
				jQuery("html,body").stop().animate({ scrollTop:curObjTop}, 200);
			}
			else{
				if(_this.zoomFlag == 0&&jQuery(window).scrollTop()<=_this.oSiderTop){
					var ttt = _this.temoScale*jQuery(document).height()-jQuery(window).height()/3-166;
				}
				else{
					var ttt = _this.temoScale*jQuery(document).height()-jQuery(window).height()/3;
				}
				jQuery(window).scrollTop(ttt);
			}
		}
		if(_this.config.showStyle == 2){
			_this.curPage = pageno;
			if(_this.config.isBuy == 0){
				if(pageno>_this.config.previewNum){
					_this.gotoPageInput.val(_this.config.previewNum+1);
					_this.curPage = _this.config.previewNum+1;
					if(jQuery(".preview_end").length>0){
						jQuery(".model").css({width:0,height:0});
						jQuery(".preview_end").show();
					}
				}
				else{
					if(jQuery(".preview_end").length>0){
						jQuery(".preview_end").hide();
					}
					_this.gotoPageInput.val(_this.curPage);
					_this.addFlash(pageno);
					_this.clearFlash(pageno);
					jQuery(".model").css({width:0,height:0});
					jQuery("#page_"+pageno).removeAttr('style');
					_this.bookMarkShow(pageno);
					if(!window.XMLHttpRequest){
						var w = jQuery("#page_"+pageno).find(".panel_inner").width();
						var h = jQuery("#page_"+pageno).find(".panel_inner").height();
						jQuery("#page_"+pageno).css({width:w,height:h});
					}
				}
			}
			else{
				_this.gotoPageInput.val(_this.curPage);
				_this.addFlash(pageno);
				_this.clearFlash(pageno);
				jQuery(".model").css({width:0,height:0});
				jQuery("#page_"+pageno).removeAttr('style');
				_this.bookMarkShow(pageno);
				if(!window.XMLHttpRequest){
						var w = jQuery("#page_"+pageno).find(".panel_inner").width();
						var h = jQuery("#page_"+pageno).find(".panel_inner").height();
						jQuery("#page_"+pageno).css({width:w,height:h});
					}
			}
			if(typeof(isPageStat)!="undefined"&&isPageStat == "true"){
				jQuery.post("/jsp_cn/test/collect.jsp", {"user_id":tmp_userId,"pid":product_id,"pageno":_this.curPage,"page_in":new Date().getTime()} );
			}
		}	
	},
	pageButton:function(pageno){
		var _this = this;
		if(pageno){
			if(_this.config.allPage == 1){
				_this.preBtn.removeClass('page_prev').addClass('page_prev_no').removeAttr('title');
				_this.nextBtn.addClass('page_next').addClass('page_next_no').removeAttr('title');

			}
			else if(pageno == 1){
				_this.preBtn.removeClass('page_prev').addClass('page_prev_no').removeAttr('title');
				_this.nextBtn.addClass('page_next').removeClass('page_next_no');
			}
			else if(pageno == _this.config.allPage){
				_this.nextBtn.removeClass('page_next').addClass('page_next_no').removeAttr('title');
				_this.preBtn.addClass('page_prev').removeClass('page_prev_no');
			}
			else{
				_this.preBtn.addClass('page_prev').removeClass('page_prev_no');
				_this.nextBtn.addClass('page_next').removeClass('page_next_no');
			}
		}
	},
	setBookMark:function(){
		var oTimer = setTimeout(function(){
				jQuery("#bookmarktips").fadeOut();
			},3000);
			jQuery("#bookmarktips").bind('mouseover',function(){
				clearTimeout(oTimer);
			});
			jQuery("#bookmarktips").bind('mouseout',function(){
				oTimer = setTimeout(function(){
					jQuery("#bookmarktips").fadeOut();
				},3000);
		});
	},
	addBookMark:function(){
		var _this = this;
		_this.btn_bookmark.bind('click',function(){
			if(jQuery(this).hasClass('book_mark_cur')){//取消标签
				jQuery.post("/app/my/bookself/delbookmark.do",{productid:_this.config.productId,bookmarkid:_this.config.bookmarkid,fromdomain:""},function(data){
					if(data == 1){
						_this.config.bookMark = 0;
						_this.btn_bookmark.removeClass('book_mark_cur');
						jQuery("#bookmarktips p").html('书签已取消');
						jQuery("#bookmarktips").unbind("click").fadeIn();
						_this.setBookMark();
					}
				});
			}
			else{
				if(_this.config.isLogin == 0){
					showlogin();
				}
				else{
					if(isZuZhi == "1"){
						mailDialogAlert(login_email,landingEmail,domailUrl);
						return;
					}
					jQuery.post("/app/my/bookself/addbookmark.do",{productid:_this.config.productId,bookmarkid:_this.config.bookmarkid,bookmark:_this.curPage,fromdomain:""},function(data){
					_this.config.bookMark = _this.curPage;
					readerConfig.bookMark = _this.curPage;
					_this.btn_bookmark.addClass('book_mark_cur');
					jQuery("#bookmarktips p").html('添加书签成功,可以进入我的书房中查看。<a title="点击查看" target="_blank" href="http://shufang.docin.com/">点击查看&gt;&gt;</a>');
					jQuery("#bookmarktips").unbind("click").fadeIn();
					jQuery("#addBookShop").removeClass('green').addClass('gray').html('<span class="ico_mini ico_addfav"></span>已收藏');
					_this.setBookMark();
					//暂时不知道返回值什么用途 估计没用
					});
					
				}
			}
		});
	},
	bookMarkShow:function(pageno){
		var _this = this;
		if(pageno == _this.config.bookMark){
			_this.btn_bookmark.addClass('book_mark_cur');
		}
		else{
			_this.btn_bookmark.removeClass('book_mark_cur');
		}
	},
	jumpToPage:function(){
		var _this = this;
		_this.gotoPageInput.bind('keydown',function(ev){
			if(ev.keyCode == 13){
				var curpage = parseInt(jQuery(this).val());
				if(isNaN(curpage)){
					return false;
				}
				if(curpage<=0||curpage>_this.config.allPage){
					return false;
				}
				_this.curPage = curpage;
				_this.pageButton(_this.curPage);
				_this.gotoPage(_this.curPage,1);
				if(jQuery('.recommedMod').length>0&&_this.config.showStyle == 2){
					jQuery(".recommed_shadow").hide();
					jQuery(".recommedMod").hide();
				}
			}
		});
	},
	readerMenu:function(){
		var _this = this;
		var menuList = jQuery(".menu_list_wrap");
		var otime = null;
		if(menuList.length<0){return false;}
		_this.menu.bind('mouseover',function(){
			clearTimeout(otime);
			menuList.show();
		}).bind('mouseleave',function(){
			otime = setTimeout(function(){
				menuList.hide();
			},100);
			
		});
		//菜单功能
		var oShowInfo = jQuery("#j_showinfo"),
		oShowAbout = jQuery("#j_about"),
		oJlist = jQuery("#j_list"),
		oJpic = jQuery("#j_pic");
		if(oShowInfo.length>0){
			oShowInfo.bind('click',function(){
				var docInfo = new CreateDocinDialog(showInfoConfig);
			});
		}
		if(oShowAbout.length>0){
			oShowAbout.bind('click',function(){
				var docAbout = new CreateDocinDialog({
					id:'docAbout',
					title:'关于豆丁',
					button:2,
					shadow:1,
					content:'<span class="fwb">豆丁网是面向全球的在线C2C文档分享网站。</span><br/>豆丁网(www.docin.com)创立于2008年，致力于打造面向全球的中文社会化阅读平台，为用户提供一切有价值的可阅读之物。经过几年的运营，豆丁网已经成功跻身互联网全球500强，成为提供垂直服务的优秀网站之一。目前豆丁网拥有分类广泛的实用文档、众多出版物、行业研究报告、以及数千位行业名人贡献的专业文件，各类读物总数超过四亿，是面向全球的中文文档库。'
				});
			});
		}
		if(oJlist.length>0){
			oJlist.bind('click',function(){
				if(jQuery(this).hasClass("cur")){return;}
				jQuery(window).unbind('resize');
				jQuery(window).unbind('scroll');
				jQuery("#contentcontainer").html("");
				_this.config.showStyle = 1;
				jQuery(".doc_reader_mod").removeClass('style_pic');
				jQuery("#maskLeft").hide();
				jQuery("#maskRight").hide();
				_this.tabInit();
				//列表模式下手型和复制放开使用
				if(jQuery("#j_hand").length>0&&jQuery("#j_select").length>0){
					jQuery("#j_hand").removeClass('drag_hand_no');
					jQuery("#j_select").removeClass('select_hand_no');
					if(jQuery('#j_comdoc').length > 0){
						jQuery("#j_hand").show(0);
						jQuery("#j_select").hide(0);
					}
				}
				if(jQuery(".preview_end").length>0){
					jQuery(".preview_end").show();
				}
				jQuery(".menu_list li").removeClass('cur');
				jQuery(this).addClass('cur');
				inpmv(5114);
			});
		}
		if(oJpic.length>0){
			oJpic.bind('click',function(){
				if(jQuery(this).hasClass("cur")){return;}
				jQuery(window).unbind('resize');
				jQuery(window).unbind('scroll');
				jQuery("#contentcontainer").html("");
				_this.config.showStyle = 2;
					if(_this.config.showAd){
						jQuery(".relative_doc_inner").height("auto");	
					}
					if(jQuery(".preview_end").length>0){
						jQuery(".preview_end").hide();
					}
					jQuery(".doc_reader_mod").addClass('style_pic');
					jQuery("#maskLeft").show();
					jQuery("#maskRight").show();
					_this.tabInit();
					//ppt模式下手型和复制都无法使用
					if(jQuery("#j_hand").length>0&&jQuery("#j_select").length>0){
						jQuery("#j_hand").addClass('drag_hand_no');
						jQuery("#j_select").addClass('select_hand_no');
						if(jQuery('#j_comdoc').length > 0){
							jQuery("#j_hand").hide(0);
							jQuery("#j_select").show(0);
						}
					}
					jQuery(".menu_list li").removeClass('cur');
					jQuery(this).addClass('cur');
					inpmv(5115);
			});
		}
	}, 
	insertAd:function(f){
		var _this = this;
		var readerAd1 = jQuery('<div id="ad1" class="adBox"><iframe src="/jsp_cn/ad/end_page.jsp?loc=1&pid='+readerConfig.productId+'" width="760" height="90" scrolling="no" frameborder="0" style="display:block;margin:0 auto;"></iframe></div>');
		var readerAd2 = jQuery('<div id="ad2" class="adBox"><iframe src="/jsp_cn/ad/end_page.jsp?loc=2" width="760" height="90" scrolling="no" frameborder="0" style="display:block;margin:0 auto;"></iframe></div>');
		var readerAd3 = jQuery('<div id="ad3" class="adBox"><iframe src="/jsp_cn/ad/end_page.jsp?loc=3" width="760" height="90" scrolling="no" frameborder="0" style="display:block;margin:0 auto;"></iframe></div>');
		var readerAd4 = jQuery('<div id="ad4" class="adBox"><iframe src="/jsp_cn/ad/end_page.jsp?loc=4" width="760" height="90" scrolling="no" frameborder="0" style="display:block;margin:0 auto;"></iframe></div>');
		var readerAd5 = jQuery('<div id="ad5" class="adBox"><iframe src="/jsp_cn/ad/end_page.jsp?loc=5" width="760" height="90" scrolling="no" frameborder="0" style="display:block;margin:0 auto;"></iframe></div>');
			if(f == 1){
				//s1 s2
				if(jQuery("#page_1").length>0&&jQuery("#ad1").length==0){
					readerAd1.insertAfter(jQuery("#page_1"));				
				}
				if(jQuery("#page_2").length>0&&jQuery("#ad2").length==0){
					readerAd2.insertAfter(jQuery("#page_2"));
				}
				if(jQuery("#page_4").length>0&&jQuery("#ad3").length==0){
					readerAd3.insertAfter(jQuery("#page_4"));
				}
				if(jQuery("#page_6").length>0&&jQuery("#ad4").length==0){
					readerAd4.insertAfter(jQuery("#page_6"));
				}
				if(jQuery("#page_8").length>0&&jQuery("#ad5").length==0){
					readerAd5.insertAfter(jQuery("#page_8"));
				}
			}
			else if(f == 2){
				//s3 s4
				if(jQuery("#page_1").length>0&&jQuery("#ad1").length==0){
					readerAd1.appendTo(jQuery("#page_1"));				
				}
				if(jQuery("#page_2").length>0&&jQuery("#ad2").length==0){
					if(_this.config.yjAdvStyle == 0){
						readerAd2.appendTo(jQuery("#page_2"));//页内
					}
					else if(_this.config.yjAdvStyle == 1){
						readerAd2.insertAfter(jQuery("#page_2"));//页外
					}
					
				}
				if(jQuery("#page_4").length>0&&jQuery("#ad3").length==0){
					readerAd3.appendTo(jQuery("#page_4"));
				}
				if(jQuery("#page_6").length>0&&jQuery("#ad4").length==0){
					readerAd4.appendTo(jQuery("#page_6"));
				}
				if(jQuery("#page_8").length>0&&jQuery("#ad5").length==0){
					readerAd5.appendTo(jQuery("#page_8"));
				}
			}
	},
	setReaderLogin:function(){
		var _this = this;
		if(_this.config.isLogin == 1){return;}
		var l = jQuery("#contentcontainer").offset().left+jQuery("#contentcontainer").width()-200;
		jQuery(".reader_login").css({left:l}).show();
		var sc = jQuery(window).scrollTop()+jQuery(window).height(),
		sct = jQuery(document.body)[0].scrollHeight;
		if(_this.config.isBuy == 0){
			var oReaderLogin = jQuery(".reader_login");
			if(oReaderLogin.length>0){
				if(_this.config.showStyle == 2){
					jQuery(".reader_login").css('bottom',40);
					return false;	
				}
				if(sc>=sct){
					jQuery(".reader_login").css('bottom',200);
				}
				else{
					jQuery(".reader_login").css('bottom',40);
				}
			}
		}
	},
	setBeforeAd:function(){//前贴片广告显示
		var _this = this;
		var boforeAd = jQuery("#beforeAd");
		if(boforeAd.length==0){return;}
		var st = jQuery(window).scrollTop();
		if(st>_this.oSiderTop){ 
			boforeAd.css({position:'fixed'});
		}
		else{
			if(_this.zoomFlag == 0){
				boforeAd.css({position:'absolute'});
			}
			else if(_this.zoomFlag == 1){
				boforeAd.css({position:'fixed'});
			}
			
		}
		if(_this.zoomFlag == 1){
			boforeAd.css({width:pageWidth});
		}
		else{
			boforeAd.css({width:playerWidth});
		}

		if(!window.XMLHttpRequest){
			boforeAd.css({position:'absolute'});
		}
	},
	countHide:function(num,numBox,o){//前贴片广告倒计时
		var boforeAd = jQuery("#beforeAd");
		if(boforeAd.length==0){return;}
		var _this = this;
		var contNum = num,
		numBox = jQuery("#"+numBox);
		numBox.html(contNum);
		var numTimer = setInterval(function(){
			if(contNum == 1){
				clearInterval(numTimer);
				jQuery("#"+o).remove();
				return;
			}
			contNum--;
			numBox.html(contNum);
		},1000);
	},
	handMove:function(){
		var oDiv=document.getElementById('contentcontainer');
		jQuery("#moveHandel").css({cursor:'url('+picture_image_path_v1+'/images_cn/news/html_reader/cursor_hand1.ico?rand=20140721),auto'});
		oDiv.onmousedown=function (ev){
		var oEvent=ev||event;	//处理事件对象兼容性问题
		var oEventY = oEvent.clientY;
		var oEventX = oEvent.clientX;
		var startY = oEventY+jQuery(window).scrollTop();
		var startX = oEventX+jQuery(window).scrollLeft();
		jQuery("#moveHandel").css({cursor:'url('+picture_image_path_v1+'/images_cn/news/html_reader/cursor_drag_hand1.ico?rand=20140721),auto'});
		if(oDiv.setCapture)
		{
			//IE
			oDiv.onmousemove=fnMove;
			oDiv.onmouseup=fnUp;	
			oDiv.setCapture();
		}
		else
		{
			//非IE
			oDiv.onmousemove=fnMove;
			oDiv.onmouseup=fnUp;
		}
		
		jQuery(".adBox").bind('mouseover',function(event){
			event.stopPropagation();
			fnUp();
		});
		// jQuery(".docin_reader_tools").bind('mouseover',function(event){
		// 	event.stopPropagation();
		// 	fnUp();
		// });

		jQuery(".addcomdocs").unbind("click").bind('click',function(event){
			event.stopPropagation();
			fnUp();
			addComDocs(this);
		});
		
		jQuery(oDiv).bind('mouseleave',function(event){
			event.stopPropagation();
			fnUp();
		});
		function fnMove(ev)
		{	
			var sct = jQuery(document).height();
			var scw = jQuery(window).width()*1.5;
			var oEvent = ev||event;
			var st = jQuery(window).scrollTop();
			var sl = jQuery(window).scrollLeft();
			var t = (oEvent.clientY+st)-startY;
			var h = (oEvent.clientX+sl)-startX;
			var cH = jQuery(window).height();
			var cW = jQuery(window).width();
			var pct = cH/sct;
			var pcw = cW/scw;
			var ddd = (t/sct)*cH/pct;
			var eee = (h/scw)*cW/pcw;
			var moveT = Math.ceil(st-ddd);
			var moveL = Math.ceil(sl-eee);
			jQuery(window).scrollTop(moveT);
			jQuery(window).scrollLeft(moveL);
			return false;
		}
		function fnUp()
		{
			jQuery("#moveHandel").css({cursor:'url('+picture_image_path_v1+'/images_cn/news/html_reader/cursor_hand1.ico?rand=20140721),auto'});
			oDiv.onmousemove=null;
			oDiv.onmouseup=null;
			if(oDiv.releaseCapture)
			{
				oDiv.releaseCapture();
			}
			return false;
		}
		return false;
	};
	},
	hideSelected:function(){
		var _this = this;
		var aFlash = document.getElementById('contentcontainer').getElementsByTagName('object');
		jQuery("#contentcontainer").bind('click',function(){
			if(_this.config.showStyle == 2){return;}
			if(jQuery("#moveHandel").length>0&&jQuery("#moveHandel").is(":visible")){
				return;
			}
			aFlash = document.getElementById('contentcontainer').getElementsByTagName('object');
			for(var i = 0;i<aFlash.length;i++){
				aFlash[i].hideSelected()
			}
			return false;
		});
		jQuery(".panel").bind("click",function(ev){
			ev.stopPropagation(); //阻止冒泡
		});
	},
	tabChangeHand:function(){
		var _this = this;
		var oHandBtn = jQuery("#j_hand"),
		oSelectBtn = jQuery("#j_select");
		if(oHandBtn.length == 0){return;}
		oHandBtn.bind('click',function(){
			if(jQuery(this).hasClass('drag_hand_no')){return;}
			jQuery(this).addClass('btn_cur');
			oSelectBtn.removeClass('btn_cur');
			jQuery("#moveHandel").show();
			if(jQuery('#j_comdoc').length > 0){
				jQuery(this).hide();
				oSelectBtn.show();
			}
			_this.handMove();
		});
		oSelectBtn.bind('click',function(){
			if(jQuery(this).hasClass('select_hand_no')){return;}
			jQuery(this).addClass('btn_cur');
			oHandBtn.removeClass('btn_cur');
			jQuery("#moveHandel").hide();
			if(jQuery('#j_comdoc').length > 0){
				jQuery(this).hide();
				oHandBtn.show();
			}
			var oDiv=document.getElementById('contentcontainer');
			oDiv.onmousedown = null;
			oDiv.onmousemove = null;
			oDiv.onmouseup = null;
		});
	},
	loadRelative:function(){
		var _this  = this;
		if(jQuery("#relative_doc_mod").length == 0){return;}
		var disease = typeof(is_disease)=="undefined"?0:1;
		var medical_id = typeof(medicalid)=="undefined"?'':medicalid;
		var dis_name = typeof(diseasename)=="undefined"?'':diseasename;
		var url = "/app/p/end/ajax/getRightRelativeDocs";
		var metaKeyWords = jQuery("meta[name='keywords']").attr("content");
		var metaDescription = jQuery("meta[name='description']").attr("content");
		var isShowAdv = _this.config.showAd;
		if(readerConfig._BFD_data == undefined){
			var iData = {medical_:medical_id,disease_name:dis_name,is_disease:disease,type:1,metaDescription:metaDescription,metaKeyWords:metaKeyWords,id:_this.config.productId,isShowAdv:isShowAdv,isType:_this.config.isType};
		}
		else{
			var iData = {medical_:medical_id,disease_name:dis_name,is_disease:disease,type:1,metaDescription:metaDescription,metaKeyWords:metaKeyWords,id:_this.config.productId,isShowAdv:isShowAdv,isBFD:readerConfig._BFD_data};
		}
		jQuery.post(url,iData,function(re){
			//将请求结果放置到相关文档div中
				jQuery("#relative_doc_mod").html(re);
				/*randmon a doc*/
				if(jQuery("#j_isend").length>0){
					var aDocLinks = jQuery("#relative_doc_mod .relative_bd .list_p a"),
						aLinksArry = [];
					if(aDocLinks.length>0){
						for(var i = 0;i<aDocLinks.length;i++){
							aLinksArry.push(jQuery(aDocLinks[i]).prop("href"));
						}
						var randomLink = Math.floor(Math.random()*aLinksArry.length);
						if(jQuery("#simiSelect").length>0){
							jQuery("#simiSelect").prop("href",aLinksArry[randomLink]);
							jQuery("#simiSelect").show();
						}
					}
				}
				if(jQuery(".sider_guanggao").length>0&&isShowAdv == true){
					jQuery(".sider_guanggao").show();
					var rightNewAd = jQuery('#right_end_new_adv'),
					oCloseCurMode = jQuery("#j_closeCurMod");
					if(rightNewAd.length>0){
						jQuery(window).bind("scroll",function(){
							var advTop = rightNewAd.offset().top,
								st = jQuery(window).scrollTop(),
								sH = st + jQuery(window).height();
								if(advTop<=sH){
									if(!jQuery('#right_end_new_adv iframe').attr('src')){
										jQuery('#right_end_new_adv iframe').attr({src:'/docin_adv/adv.do?pos=27'});
									}
								}
						});
					}
					if(oCloseCurMode.length>0&&jQuery(".sider_guanggao").length>0){
						oCloseCurMode.hover(function(){
							jQuery(".sider_guanggao").addClass("gg_hover");
						},function(){
							jQuery(".sider_guanggao").removeClass("gg_hover");
						});
						oCloseCurMode.bind("click",function(){
							if(_this.config.isLogin == 0){//未登录,放个cookie
								
								var cook = new CookieClass();
								cook.expires = 1;
								cook.domain = ".docin.com";
								cook.path = "/";
								cook.setCookie("vip_alert_adv", "1");
								
								changeCookieValue(cook,"vip_alert_adv",1);
								
								//showlogin();
								return false;
							}
							var openVipAlert = new CreateDocinDialog({
								id:"openVipTips",
								cls:"",
								title:"豆丁提示",
								content:'<p style="text-align:center;font-size:16px;font-weight:bold;height:32px;line-height:32px;margin-bottom:10px;">开通VIP，即享文档页零广告<p><p>开通VIP即享：阅读文档页零广告，还有可复制文档文本、下载免费文档不限量、专属妹子客服…等诸多<a style="text-decoration:underline;" href="http://www.docin.com/helpcenter/getProblem.do?pid=96" target="_blank" title="VIP特权">VIP特权</a><p>',
								cancel:{txt:"关闭",value:true},
								confirm:{txt:"开通VIP",value:true},
								callBack:{okBack:function(){openVipAlert = null;window.open('/app/jump_adv/pay_vip.do?channelid=0313&buyVipFrom=11');return true;},noBack:function(){openVipAlert = null;return false;}}
							});
						});
					}
				}
				relateDocH = jQuery('.relative_doc_inner').length == 1 ? jQuery('.relative_doc_inner').outerHeight() : jQuery('.relative_doc_inner').eq(1).outerHeight();				initRelaHeight();
				jQuery(window).bind('resize',function(){
					resizeRela();
				});
				if(!window.XMLHttpRequest){return;}			
		});
	},
	setFloderBtn:function(){
		var _this = this;
		var oLeft = jQuery(".main").offset().left+jQuery(".main").width();
		_this.floderBtn.css({left:oLeft});
	},
	setSearch:function(f){	//search平移
		var _this = this;
		if(jQuery('#layoutSearch').length == 0){return;}
		var oTopSearch = jQuery(".top_nav_wrap .mini_search_wrap");
		var iSearchWidth1 = _this.config.isLogin?190:250;
		var iSearchWidth2 = _this.config.isLogin?146:206;

		if(f == 1){//下移
			//oTopSearch.stop();
			// oTopSearch.animate({ 
			//     top:50
			//   },300,function(){
			//   		oTopSearch.hide();
			//   		jQuery("#topsearch").width(0);
			// 		jQuery(".top_nav_wrap .mini_search_wrap .search_in").width(0);
			//   		jQuery('#layoutSearch').show();
			// 		jQuery("#layoutSearch .search_in").width(iSearchWidth1);
			// 		jQuery("#topsearch2").width(iSearchWidth2);
			//   });
			  		jQuery('#layoutSearch').show();
	
		}else if(f == 0){//上移
				// jQuery(".top_nav_wrap .mini_search_wrap").stop();
				// jQuery(".top_nav_wrap .mini_search_wrap").show();
				// jQuery("#layoutSearch .search_in").width(0);
				// jQuery("#topsearch2").width(0);
				// jQuery('#layoutSearch').hide();
				// jQuery(".top_nav_wrap .mini_search_wrap").animate({top:0},300,function(){
				// 	jQuery(".top_nav_wrap .mini_search_wrap .search_in").width(380);
				// 	jQuery("#topsearch").width(335);
				// });
				jQuery('#layoutSearch').hide();
		}
	},
	setPage:function(window){
		var _this = this;
		var oSider = jQuery(".aside"),
		oToolBar = jQuery('.docin_reader_tools'),
		oSiderTop = _this.oSiderTop;
		oContWidth = jQuery("#contentcontainer").width(),
		oSiderRight = (jQuery(window).width()-pageWidth)/2,
		oUserInfo = jQuery('.user_doc_mod');
		var oNewDownloadBtn = jQuery(".downloadArea2");
		var oRelative = jQuery('.relative_doc_mod');
		var oFixControl = jQuery(".j_sidercontrol_build");//建筑新增代码

		if(oRelative.length>0){
			var oRelativeTop = oRelative.offset().top;
		}
		
		var oBanner = jQuery(".doc_hd_mini"),
			oBannerBuild = jQuery(".doc_hd");
		if(oBanner.length>0){
			var oBannerTop = oBanner.height();
		}
		if(oBannerBuild.length>0){
			var oBannerTop = oBannerBuild.height() + jQuery(".doc_hds").height();
		}

		setToolBarFix(1);
		siderFixedMove(1);
		_this.width = jQuery(window).width();
		_this.height = jQuery(window).height();
		jQuery(window).bind('scroll',function(){
			if((_this.zoomFlag == 0||_this.zoomFlag == 1)||_this.config.showStyle == 1){
				setBarPosition();
			}
			if(!window.XMLHttpRequest){return;}
			if(_this.zoomFlag == 1||_this.zoomFlag == 2){
				setToolBarFix(2);
				siderFixedMove(2);
			}
			else if(_this.zoomFlag == 0&&_this.config.showStyle == 1){
				setToolBarFix(1);
				siderFixedMove(1);
				_this.setBeforeAd();
				if(_this.curPage == 1){
					jQuery('.reader_container .panel_inner').eq(0).css("backgroundPosition","center center");
				}
			}
			else if(_this.zoomFlag == 0&&_this.config.showStyle==2){
				setToolBarFix(1);
			}
			if(_this.zoomFlag == 0&&_this.config.showStyle == 1 && (typeof(isCartOrisCode)=="undefined"||typeof(isBuilding)=="undefined")){
				var st = jQuery(window).scrollTop();
				if(st>oSiderTop){
					resizeRelaHeight();				
				}
				else{
					initRelaHeight();
				}
			}
		});
		jQuery(window).bind('resize',function(){
			if(!window.XMLHttpRequest){return;}
			oSiderRight = (jQuery(window).width()-pageWidth)/2;
			if(typeof(isBuilding)!="undefined"){
				_this.setFloderBtn();//建筑新增代码
			}
			if(jQuery(window).scrollTop()>oSiderTop){

				if(_this.zoomFlag == 0&&_this.config.showStyle == 1){//初始化状态
					siderFixedMove(1);
					return false;
				}
				if(_this.zoomFlag == 1||_this.zoomFlag == 2){
					siderFixedMove(2);
					return false;
				}
			}
			else{
				if(_this.zoomFlag == 0){//初始化状态
					siderFixedMove(3);
					return false;
				}
				if(_this.zoomFlag == 1||_this.zoomFlag == 2){
					siderFixedMove(2);
					return false;
				}
			}
			
		});
		function siderFixedMove(f){
			if(!window.XMLHttpRequest){return;}
			var st = jQuery(window).scrollTop();
			oSiderRight = (jQuery(window).width()-pageWidth)/2;


			//建筑终极页新增代码开始
			if(typeof(isBuilding)!="undefined"&&isBuilding ==1){
				var cH = jQuery(window).height(),
				posb = oFixControl.offset().top+oFixControl.height(),
				sth = cH + st,
				post = oFixControl.offset().top;
				if(f == 1){
					if(st >oSiderTop){
						if(oSiderRight<=0){
							oSider.css({margin:0});
						}
						else{
							oSider.css({margin:0});
						}
					}
				}
				else if(f == 2){
					if(oSiderRight<=0){
						oSider.css({margin:0});
					}
					else{
						oSider.css({margin:0});
						oSiderRight = (jQuery(window).width()-pageWidth)/2;
					}
				}
				if(oUserInfo.length>0){
					oUserInfo.show();
					}
				if(oNewDownloadBtn.length > 0){
					oNewDownloadBtn.show();
				}
				if(_this.config.showStyle == 1&&_this.zoomFlag == 0){
					relativeControl();
				}
				return false;
			}
			//建筑终极页新增代码结束


			if(f == 1){ 
				if(st >oSiderTop){
					if(oSiderRight<=0){
						oSider.css({position:'fixed',top:56,right:oSiderRight*2,left:'auto',margin:0});
					}
					else{
						oSider.css({position:'fixed',top:56,right:oSiderRight,left:'auto',margin:0});
					}
					if(typeof(isCartOrisCode)!="undefined"&&isCartOrisCode ==1){
						if(oUserInfo.length>0){
							oUserInfo.show();
						}
					}
					else{
						if(oUserInfo.length>0){
							oUserInfo.hide();
						}
						if(oNewDownloadBtn.length > 0){
							oNewDownloadBtn.hide();
						}
					}
				}
				else{
					oSider.css({position:'relative',top:0,right:0,left:'auto',marginTop:0});
					if(oUserInfo.length>0){
							oUserInfo.show();
						}
					if(oNewDownloadBtn.length > 0){
						oNewDownloadBtn.show();
					}
				}
			}
			else if(f == 2){
				if(oUserInfo.length>0){
					oUserInfo.hide();
				}
				if(oNewDownloadBtn.length > 0){
					oNewDownloadBtn.hide();
				}
				if(oSiderRight<=0){
					oSider.css({position:'fixed',top:56,right:oSiderRight*2,left:'auto',margin:0});
				}
				else{
					oSider.css({position:'fixed',top:56,margin:0,left:'auto'});
					oSiderRight = (jQuery(window).width()-pageWidth)/2;
					oSider.css({right:oSiderRight});
				}
				return;
				
			}
			else if(f == 3){
				oSider.css({position:'relative',top:0,right:0,left:'auto',marginTop:0});
				return;
			}
		}
		function relativeControl(){
			if(!window.XMLHttpRequest){return;}
			var st = jQuery(window).scrollTop();
			cH = jQuery(window).height(),
			posb = oFixControl.offset().top+oFixControl.height(),
			sth = cH + st,
			post = oFixControl.offset().top;
			var oFixTop = oFixControl.offset().top;
			if(st>=_this.oSiderTop){
				if(oFixControl.height()<=cH){
					oFixControl.css({position:'fixed',top:50,bottom:'auto'});
					return;
				}
				if(st>sst){//向下
					if(ft>=0){
						//1、先判断右侧内容看到最后，即让他居底部定位
						posb = oFixControl.offset().top+oFixControl.height();
						if(posb<=sth){
							oFixControl.css({position:'fixed',bottom:0,top:'auto'});
							oFixTop = oFixControl.offset().top;
							ftup = oFixTop - 140;
						}
						else{
							oFixControl.css({position:'relative',bottom:'auto',top:ft});
						}
					}
				}
				else{//向上
					if(ftup>=0){
						if(oFixTop>=st){
							oFixControl.css({position:'fixed',top:50,bottom:'auto'});
							oFixTop = oFixControl.offset().top;
							ft = oFixTop - 140;
						}
						else{
							oFixControl.css({position:'relative',top:ftup,bottom:'auto'});
						}
						
					}
				}

			}
			else{
				oFixControl.css({position:'relative',top:0,bottom:'auto'});
				ft = ftup = 0;
			}
			sst = st;
		}
		function setToolBarFix(f){
			if(!window.XMLHttpRequest){return;}
			var st = jQuery(window).scrollTop();
			if(f == 1){
				if(st >= oBannerTop){
					oToolBar.css({top:0});
					if(_this.zoomFlag == 0){
						jQuery("#bookmarktips").css({top:41,width:800});
					}
					else{
						jQuery("#bookmarktips").css({top:41,width:'auto'});
					}
					_this.setSearch(1);
					return;
				}
				else{
					var tt1 = jQuery("#contentcontainer").offset().top-60;
					jQuery("#bookmarktips").css({top:tt1,width:800});
					oToolBar.css({top:oBannerTop-st});
					if(st == 0){
						_this.setSearch(0);
					}
				}
			}
			else if(f == 2){
				oToolBar.css({top:0});
				_this.setSearch(1);
				jQuery("#bookmarktips").css({top:41,width:'auto'});
					return;				
			}
		}
		window.setToolBarFix = setToolBarFix;
		window.siderFixedMove = siderFixedMove;
		_this.relativeControl = relativeControl;
	},
	pageScroll:function($){
		var _this = this;
		$.fn.scrollLoading = function(options) {
		var defaults = {
			attr: "data-url"	
		};
		var params = $.extend({}, defaults, options || {});
		params.cache = [];
		$(this).each(function() {
			var node = this.nodeName.toLowerCase(), url = $(this).attr(params["attr"]);
			//if (!url) { return; }
			//重组
			var data = {
				obj: $(this),
				tag: node,
				url: url
			};
			params.cache.push(data);
		});
		
		//动态显示数据
		var loading = function() {
			var st = $(window).scrollTop(), sth = st + $(window).height();
			var ddd = [];//记录页码
			$.each(params.cache, function(i, data) {
				var o = data.obj, tag = data.tag, url = data.url;
				if (o) {

						post = o.offset().top; posb = post + o.height();

						var a = (post > st) && ((post-st)<$(window).height()/2);
						var b = (posb < sth)&&((posb-st)>$(window).height()/2);
						//var c = (post<sth)&&(posb>sth)&&(docinReader.curPage == 1);
						if(a||b){
							var t = i+1;
							// ddd.push(t);
							// console.log(ddd);
							if (tag === "img") {
								//图片，改变src
								o.attr("src", url);	
							}else {
									// o.load(url);	
								if(log_preNum != t){
									log_preNum = t;
									_this.addFlash(t);
									_this.clearFlash(t);
									jQuery('.page_cur').val(t);
									_this.curPage = t;
									_this.pageButton(t);
									_this.bookMarkShow(t);
									if(typeof(isPageStat)!="undefined"&&isPageStat == "true"){
										jQuery.post("/jsp_cn/test/collect.jsp", {"user_id":tmp_userId,"pid":product_id,"pageno":t,"page_in":new Date().getTime()} );
									}
									if(_this.config.isBuy == 1){
										if(typeof fromWebSearch !="undefined"&&_this.curPage == _this.config.allPage){

											fromWebSearch();
										}
									}
									else if(_this.config.isBuy == 0){
										if(typeof fromWebSearch !="undefined"&&_this.curPage == _this.config.previewNum){
											fromWebSearch();
										}
									}
								}

							}
						}
				}
			});	
			scrollFlag = true;
			return false;	
		};
		//事件触发
		//加载完毕即执行
		if(_this.curPage == 1||_this.curPage ==readerConfig.previewNum){
			loading();
		}
		//loading();
		//滚动执行
		$(window).bind("scroll",function(){
			if(_this.config.showStyle == 1){
				if(scrollFlag == true){
					scrollFlag = false;
					loading();
				}
				
				//setTimeout(function(){loading();},100);
			}

			// if(docinReader.config.showStyle == 1&&scrollFlag == true){
			// 	setTimeout(function(){loading();},100);
			// 	scrollFlag = false;
			// }
		});

	};
	}
}
//页面停留时间

//var pageTime = [[1,1445396823336]];

function PageTiming(){
	this.init();
}
PageTiming.prototype={
	init:function(){
		var me = this;
		me.visit_timing(); 
		me.btnClick();
	},
	visit_timing:function(){//终极页停留时间
		window.onbeforeunload = function() {
			var outTime = new Date().getTime();
	    	jQuery.post("/jsp_cn/test/collect.jsp", {"user_id":tmp_userId,"pid":product_id,"out":outTime} );
		};
	},
	btnClick:function(){
		//顶部收藏
		if(jQuery("#addBookShop").length>0){
			jQuery("#addBookShop").bind("click",function(){
				var scTime = new Date().getTime();
				jQuery.post("/jsp_cn/test/collect.jsp", {"userid":tmp_userId,"pid":product_id,"sc":scTime} );
			});
		}
		//顶部下载
		if(jQuery("#docDownload").length>0){
			jQuery("#docDownload").bind("click",function(){
				var xzTime = new Date().getTime();
				jQuery.post("/jsp_cn/test/collect.jsp", {"userid":tmp_userId,"pid":product_id,"xz":xzTime} );
			});
		}
		//右侧展开
		if(jQuery("#openAll").length>0){
			jQuery("#openAll").bind("click",function(){
				var docTime = new Date().getTime();
				jQuery.post("/jsp_cn/test/collect.jsp", {"userid":tmp_userId,"pid":pid,"doc":docTime} );
			});
		}
		//右侧相关文档
		if(jQuery("#relative_doc_mod").length>0){
			jQuery("#relative_doc_mod").bind("click",function(){
				var xgwdTime = new Date().getTime();
				jQuery.post("/jsp_cn/test/collect.jsp", {"userid":tmp_userId,"pid":product_id,"xgwd":xgwdTime} );
			});
		}
		//文字选择图标
		if(jQuery("#j_select").length>0){
			jQuery("#j_select").bind("click",function(){
				var wzTime = new Date().getTime();
				jQuery.post("/jsp_cn/test/collect.jsp", {"userid":tmp_userId,"pid":product_id,"wz":wzTime} );
			});
		}
		//喜欢该文档的用户还喜欢
		if(jQuery("#jControlDiv").length>0){
			jQuery("#jControlDiv").bind("click",function(){
				var hxhTime = new Date().getTime();
				jQuery.post("/jsp_cn/test/collect.jsp", {"userid":tmp_userId,"pid":product_id,"hxh":hxhTime});
			});
		}
		}
}


var docinReader = new DocinReaderPlayer(readerConfig);
//开启页面统计start
if(typeof(isPageStat)!="undefined"&&isPageStat == "true"){
	var pageTiming = new PageTiming();
}
//开启页面统计end
(function(){
	var oBtnShowAll = jQuery("#openAll"),
	oInfoArea = jQuery(".user_doc_bd"),
	oShowTxt = jQuery("#showTxt"),
	oshowBtn = jQuery("#openAll .ico_arrow"),
	oMoreDec = jQuery("#moreDec"),
	oElipDec = jQuery("#elipDec"),
	oModelArea = jQuery(".user_doc_mod");
	var iTopicConfig = {};
	var topicConfig = {
		id:"j_topic",
		cls:"topicDialog",
		button:1,
		title:" ",
		cancel:{txt:"取消",value:false},
		confirm:{txt:"创建专题，我来当总编",value:true},
		content:'<p>以下<span class="topicNum"></span>个专题都包含此篇文档</p><div id="topicListMod"></div>',
		callBack:{okBack:function(){window.open("/topic/preaddtopic.do","_blank");}}


	};
	if(oShowTxt.length>0){
		oBtnShowAll.bind('click',function(){
			if(jQuery(".user_doc_mod .inner").hasClass('innerShow')){
				closeMoreHandel();
				oshowBtn.removeClass("ico_arrow_up");
			}
			else{
				jQuery(".user_doc_mod .inner").addClass('innerShow');
				oShowTxt.html('收起');
				oshowBtn.addClass("ico_arrow_up");
				if(jQuery.isEmptyObject(iTopicConfig)){
				jQuery.ajax({
					url:"/app/p/end/ajax/getDocDetailExtends?id="+readerConfig.productId,
					success:function(data){
						iData = JSON.parse(data);
						iTopicConfig = iData;
						if(iData.stats == "10010"){
					if(iData.total == 0){return;}
					if(jQuery(".info_list").length > 0){
						jQuery('<dt>入选专题：</dt><dd><a id="openTopic" href="javascript:void(0);" onmousedown="return inpmv(5959)">'+iData.total+'</a>个</dd>').appendTo(jQuery(".info_list"));
						jQuery("#openTopic").on("click",function(){
							var topicDialog = new CreateDocinDialog(topicConfig);
							doWidthData(iData);
						});
					}
				}
					}
				});
			}
			else{
				doWidthData(iTopicConfig);
			}
			}
		});
		function closeMoreHandel(){
			jQuery(".user_doc_mod .inner").removeClass('innerShow');
			oShowTxt.html('详情');
		}

		function doWidthData(iData){
			jQuery("#j_topic .topicNum").html(iData.total);
			if(iData.total <= 5){
					var oUl = jQuery('<ul class="topicList clear"></ul>').appendTo(jQuery("#topicListMod"));
					var aItems = iData.items;
					for(var i = 0;i<iData.items.length;i++){
						var iJing = aItems[i].if_fine?' <span title="精华专题" class="ico_jing"></span>':'';
						jQuery('<li><a href="http://www.docin.com/t-'+aItems[i].topic_id+'.html" target="_blank" title="'+aItems[i].topic_title+'">'+aItems[i].short_title+'</a>'+iJing+'</li>').appendTo(oUl);
					}
					jQuery(".topicList").show();
					jQuery(".tabBtn .pre").addClass("default");
					jQuery(".tabBtn .next").addClass("default");
				}
				else{
					var iTabIndex = 0;
					var iPage = Math.ceil(iData.total/5);
					var aItems = iData.items;
					for(var j = 0;j<iPage;j++){
						var oUl = jQuery('<ul id="t'+j+'" class="topicList"></ul>').appendTo(jQuery("#topicListMod"));
						if(j == iPage-1){
							var mol = iData.total%5;
							var t = mol?j*5+mol:(j+1)*5;
						}else{
							var t = (j+1)*5;
						}

						for(var i = j*5;i<t;i++){
							var iJing = aItems[i].if_fine?' <span title="精" class="ico_jing"></span>':'';
							jQuery('<li><a href="http://www.docin.com/t-'+aItems[i].topic_id+'.html" target="_blank" title="'+aItems[i].topic_title+'">'+aItems[i].short_title+'</a>'+iJing+'</li>').appendTo(oUl);
						}
		
					}
					jQuery("#t"+iTabIndex).show();
					jQuery('<div class="tabBtn"><a class="pre default" href="javascript:void(0);">&lt;</a><a class="next" href="javascript:void(0);">&gt;</a></div>').appendTo(jQuery("#topicListMod"));
					jQuery(".tabBtn a.next").on("click",function(){
						iTabIndex++;
						setPageBtn();
						if(iTabIndex >= iPage){
							iTabIndex = iPage-1;
							return;
						}
						jQuery(".topicList").hide();
						jQuery("#t"+iTabIndex).show();
					});
					jQuery(".tabBtn a.pre").on("click",function(){
						iTabIndex--;
						setPageBtn();
						if(iTabIndex < 0){
							iTabIndex = 0;
							return;
						}
						jQuery(".topicList").hide();
						jQuery("#t"+iTabIndex).show();
					});
					function setPageBtn(){
						if(iTabIndex <= 0){
							jQuery(".tabBtn .pre").addClass("default");
							jQuery(".tabBtn .next").removeClass("default");
						}
						else if(iTabIndex >= iPage-1){
							jQuery(".tabBtn .pre").removeClass("default");
							jQuery(".tabBtn .next").addClass("default");
						}
						else{
							jQuery(".tabBtn .pre").removeClass("default");
							jQuery(".tabBtn .next").removeClass("default");
						}
					}
				}
		}
	}
	else{
		if(oBtnShowAll.length>0){
			oBtnShowAll.bind('click',function(){
				if(jQuery(".user_doc_mod .inner").hasClass('innerShow')){
					closeMoreHandel();
				}
				else{
					jQuery(".user_doc_mod .inner").addClass('innerShow');
					if(oMoreDec.length>0){
						oElipDec.hide();
						oMoreDec.show();
					}
					oBtnShowAll.html('收起');
				}
			});
			function closeMoreHandel(){
				jQuery(".user_doc_mod .inner").removeClass('innerShow');
				if(oMoreDec.length>0){
					oMoreDec.hide();
					oElipDec.show();
				}
				oBtnShowAll.html('文档详情');
			}
		}
	}
})();
function resizeRela(){
	var cH = jQuery(window).height();//屏幕高度
	if(cH < 310){
		return false;
	}
	if(jQuery(".sider_guanggao").length > 0){
		var adHeight = jQuery(".sider_guanggao").height();
	}
	else{
		var adHeight = 0;//vip
	}
	if(jQuery(window).scrollTop()>docinReader.oSiderTop){
		//var activeHeight = cH - 56 - 145 -10;//相关文档高度
		// var activeHeight = cH - adHeight -56;
		// if(relateDocH > activeHeight){
		// 	jQuery(".relative_doc_inner").css({height:activeHeight-2});
		// }
		// else{
		// 	jQuery(".relative_doc_inner").css({height:'auto'});
		// }
		resizeRelaHeight();
	}
	else{
		initRelaHeight();
	}
}

function initRelaHeight(){
	jQuery('.relative_hot_class_mod').show();
	var cH = jQuery(window).height();
	if(cH < 310){
		return false;
	}
	var initActiveHeight = cH - oRelaTop;
	if(jQuery(".relative_bd .list_item").length>0){
		var oItemHeight = jQuery(".relative_bd .list_item").eq(1).outerHeight();
		var showNum = Math.floor((initActiveHeight-40-jQuery(".relative_bd .list_item .item_show").outerHeight())/oItemHeight);
		initActiveHeight = showNum*oItemHeight+40+jQuery(".relative_bd .list_item .item_show").outerHeight();
	}
	if(relateDocH > initActiveHeight){
		if(relateDocH < (jQuery(".relative_bd .list_item").eq(1).outerHeight()*2 +jQuery(".relative_bd .list_item .item_show").outerHeight())){
			var iH = jQuery(".relative_bd .list_item").eq(1).outerHeight()*2 +jQuery(".relative_bd .list_item .item_show").outerHeight();
			jQuery('.relative_doc_inner').length == 1 ? jQuery('.relative_doc_inner').css({height:iH}) : jQuery('.relative_doc_inner').eq(1).css({height:iH});
		}else{
			jQuery('.relative_doc_inner').length == 1 ? jQuery('.relative_doc_inner').css({height:initActiveHeight-2}) : jQuery('.relative_doc_inner').eq(1).css({height:initActiveHeight-2});		}
	}
	else{
		jQuery('.relative_doc_inner').length == 1 ? jQuery('.relative_doc_inner').css({height:'auto'}) : jQuery('.relative_doc_inner').eq(1).css({height:'auto'});	}
}
function resizeRelaHeight(){
	jQuery('.relative_hot_class_mod').hide();
	var cH = jQuery(window).height();
	if(cH < 310){
		return;
	}
	if(jQuery(".sider_guanggao").length > 0){
		var adHeight = jQuery(".sider_guanggao").height();
	}
	else{
		var adHeight = 0;//vip
	}
	var activeHeight = cH - adHeight -56;
	if(jQuery(".relative_bd .list_item").length>0){
		var oItemHeight = jQuery(".relative_bd .list_item").eq(1).outerHeight();
		var showNum = Math.floor((activeHeight-40-jQuery(".relative_bd .list_item .item_show").outerHeight())/oItemHeight);
		activeHeight = showNum*oItemHeight+40+jQuery(".relative_bd .list_item .item_show").outerHeight();
	}
	if(relateDocH > activeHeight){
		if(relateDocH < (jQuery(".relative_bd .list_item").eq(1).outerHeight()*2 +jQuery(".relative_bd .list_item .item_show").outerHeight())){
			var iH = jQuery(".relative_bd .list_item").eq(1).outerHeight()*2 +jQuery(".relative_bd .list_item .item_show").outerHeight();
			jQuery('.relative_doc_inner').length == 1 ? jQuery('.relative_doc_inner').css({height:iH}) : jQuery('.relative_doc_inner').eq(1).css({height:iH});
		}else{
			jQuery('.relative_doc_inner').length == 1 ? jQuery('.relative_doc_inner').css({height:activeHeight-2}) : jQuery('.relative_doc_inner').eq(1).css({height:activeHeight-2});
		}
	}
	else{
		jQuery('.relative_doc_inner').length == 1 ? jQuery('.relative_doc_inner').css({height:'auto'}) : jQuery('.relative_doc_inner').eq(1).css({height:'auto'});
	}
}

function hideOtherSelected(){
	var aFlash = document.getElementById('contentcontainer').getElementsByTagName('object');
	for(var i = 0;i<aFlash.length;i++){
		aFlash[i].hideSelected();
	}
}

function likeTooModControl(f){
	if(f == 1){
		if(jQuery('#jControlDiv').length>0){
			jQuery('#jControlDiv').show();
		}
		else if(jQuery("#likeToo").length>0){
			jQuery("#likeToo").show();
		}
	}
	else if(f == 2){
		if(jQuery('#jControlDiv').length>0){
			jQuery('#jControlDiv').hide();
		}
		else if(jQuery("#likeToo").length>0){
			jQuery("#likeToo").hide();
		}
	}
}
function controlLeftBottomAdv(f){
	if(jQuery(".left_buttom_adv").length == 0){return;}
	if(f == 1){
		jQuery(".left_buttom_adv").show();
	}
	else if(f == 2){
		jQuery(".left_buttom_adv").hide();
	}
}
//是否有权限复制
function isAuthForCopy(){
		if(typeof readerConfig.isBD == "undefined"){return 0;}
		if(readerConfig.isBD == 0){
			if(readerConfig.isLogin == 0){//未登录
				//放个cookie
				var cook = new CookieClass();
				cook.expires = 1;
				cook.domain = ".docin.com";
				cook.path = "/";
				cook.setCookie("can_copy_alert", "1");
				changeCookieValue(cook,"can_copy_alert");
				
				return 0;
			}
			else if(readerConfig.showAd == true){
				var openVipAlert = new CreateDocinDialog({
					id:"openVipTips",
					cls:"",
					title:"豆丁提示",
					content:'<p style="text-align:center;font-size:16px;font-weight:bold;height:32px;line-height:32px;margin-bottom:10px;">您还不是VIP，不能使用复制功能<p><p>开通VIP即享：可复制文档文本，还有阅读文档页零广告、下载免费文档不限量、专属妹子客服…等诸多<a style="text-decoration:underline;" href="http://www.docin.com/helpcenter/getProblem.do?pid=96" target="_blank" title="VIP特权">VIP特权</a><p>',
					cancel:{txt:"关闭",value:true},
					confirm:{txt:"开通VIP",value:true},
					callBack:{okBack:function(){openVipAlert = null;window.open('http://www.docin.com/app/jump_adv/pay_vip.do?channelid=0313&buyVipFrom=12');return true;},noBack:function(){openVipAlert = null;return false;}}
				});
				
				
				
				return 0;
			}
			else if(readerConfig.showAd == false){
				return 1;
			}
		}
		return 1;
	}



	function checkCookie_vip(){
		var cook = new CookieClass();
		var remindClickId = cook.getCookie("vip_alert_adv");
		if(remindClickId == "1" && tmp_userId > 0 && isAdvShow == "true"){//不是vip，显示弹层
			
			var openVipAlert = new CreateDocinDialog({
				id:"openVipTips",
				cls:"",
				title:"豆丁提示",
				content:'<p style="text-align:center;font-size:16px;font-weight:bold;height:32px;line-height:32px;margin-bottom:10px;">开通VIP，即享文档页零广告<p><p>开通VIP即享：阅读文档页零广告，还有可复制文档文本、下载免费文档不限量、专属妹子客服…等诸多<a style="text-decoration:underline;" href="http://www.docin.com/app/jump_adv/pay_vip.do?channelid=0313&buyVipFrom=11" target="_blank" title="VIP特权">VIP特权</a><p>',
				cancel:{txt:"关闭",value:true},
				confirm:{txt:"开通VIP",value:true},
				callBack:{okBack:function(){openVipAlert = null;window.open('/app/jump_adv/pay_vip.do?channelid=0313&buyVipFrom=11');return true;},noBack:function(){openVipAlert = null;return false;}}
			});
		}
		
		cook.expires = 1;
		cook.domain = ".docin.com";
		cook.path = "/";
		cook.setCookie("vip_alert_adv", -1);
	}
	function checkCookie_copy(){
		var cook = new CookieClass();
		var remindClickId = cook.getCookie("can_copy_alert");
		if(remindClickId == "1" && tmp_userId > 0 && isAdvShow == "true"){//不是vip，显示弹层
			
			var openVipAlert = new CreateDocinDialog({
				id:"openVipTips",
				cls:"",
				title:"豆丁提示",
				content:'<p style="text-align:center;font-size:16px;font-weight:bold;height:32px;line-height:32px;margin-bottom:10px;">您还不是VIP，不能使用复制功能<p><p>开通VIP即享：可复制文档文本，还有阅读文档页零广告、下载免费文档不限量、专属妹子客服…等诸多<a style="text-decoration:underline;" href="http://www.docin.com/helpcenter/getProblem.do?pid=96" target="_blank" title="VIP特权">VIP特权</a><p>',
				cancel:{txt:"关闭",value:true},
				confirm:{txt:"开通VIP",value:true},
				callBack:{okBack:function(){openVipAlert = null;window.open('http://www.docin.com/app/jump_adv/pay_vip.do?channelid=0313&buyVipFrom=12');return true;},noBack:function(){openVipAlert = null;return false;}}
			});
		}
		cook.expires = 1;
		cook.domain = ".docin.com";
		cook.path = "/";
		cook.setCookie("can_copy_alert", -1);
	}
	checkCookie_vip();
	checkCookie_copy();

function flashChecker(){
	var hasFlash=0;		//是否安装了flash
	var flashVersion=0;		//flash版本

	if(document.all){
		var swf = new ActiveXObject('ShockwaveFlash.ShockwaveFlash');
		if(swf) {
			hasFlash=1;
			VSwf=swf.GetVariable("$version");
			flashVersion=parseInt(VSwf.split(" ")[1].split(",")[0]);
		}
	}else{
		if (navigator.plugins && navigator.plugins.length > 0){
			var swf=navigator.plugins["Shockwave Flash"];
			if (swf){

				hasFlash=1;
				var words = swf.description.split(" ");

				for (var i = 0; i < words.length; ++i){
			         if (isNaN(parseInt(words[i]))) continue;
			         flashVersion = parseInt(words[i]);
				}
			}
		}
	}
	return {f:hasFlash,v:flashVersion};
}

(function(){
	var fls=flashChecker();
	if(fls.f){
		//console.log("您安装了flash,当前flash版本为: "+fls.v+".x")
	}else{
		jQuery('<div id="unFlashTips" style="position:absolute;left:0;top:0;z-index:2;width:100%;height:200px;padding-top:160px;text-align:center;background:url('+picture_image_path_v1+'/images_cn/error/error-flash-tips.jpg) no-repeat 50% 70px">您的计算机尚未安装Flash，<a href="https://get.adobe.com/cn/flashplayer/" target="_blank">点击安装https://get.adobe.com/cn/flashplayer/</a></div>').appendTo('.doc_reader_mod');
	}
}())

/* 加入组合文档 */
function addComDocs (obj, bs){

	var bn = jQuery(obj).hasClass('addcomdocs-on');

	if(bn){

		var productId = readerConfig.productId,
			pageCur = jQuery(obj).parent('.model').index()+1;

		jQuery.ajax({
			type:'GET',
			url:"/p/end/ajax/addUserComDocProduct.do?pid="+productId+"&page="+pageCur,
			/**
			 *	-1:用户未登陆，失败
			 *  -2:参数错误，失败
			 *  -3:文档不存在,失败
			 *  -4:传来的页码范围超出改文档最大页码 失败
			 *  -5:文档不允许添加到组合文档，失败
			 *  -6:数据异常，失败
			 *  -7:个数已经达到上限，失败
			 *  -8：已经添加过在列表中了，失败
			 *  正数:成功，返回值为添加后组合列表个数
			 *  -10：非vip用户不能操作
			 */
			success:function(data){
				if(data > 0){
					comDocsAnimate(data);
					comdocsConf.zhPages+=','+pageCur+'';
					comdocsConf.zhtotal++;
				}else{
					if(data == -8){
						comDocsAnimate();
					}else if(data == -7){
						comDocsUpNum();
					}else if(data == -6){
						alert('数据异常，失败');
					}else if(data == -5){
						alert('文档不允许添加到组合文档，失败');
					}else if(data == -4){
						alert('传来的页码范围超出改文档最大页码');
					}else if(data == -3){
						alert('文档不存在,失败');
					}else if(data == -2){
						alert('参数错误，失败');
					}else if(data == -1){
						alert('用户未登陆，失败');
					}else if(data == -10){
						var openVipAlert = new CreateDocinDialog({
							id:"openVipTips",
							cls:"",
							title:"豆丁提示",
							content:'<p style="text-align:center;font-size:16px;font-weight:bold;height:32px;line-height:32px;margin-bottom:10px;">开通VIP，即享文档页零广告<p><p>开通VIP即享：阅读文档页零广告，还有可复制文档文本、下载免费文档不限量、专属妹子客服…等诸多<a class="dialog_closed_by_link" style="text-decoration:underline;" href="http://www.docin.com/helpcenter/getProblem.do?pid=96" target="_blank" title="VIP特权">VIP特权</a><p>',
							cancel:{txt:"关闭",value:true},
							confirm:{txt:"开通VIP",value:true},
							callBack:{okBack:function(){openVipAlert = null;window.open('/app/jump_adv/pay_vip.do?channelid=0313&buyVipFrom=11');return true;},noBack:function(){openVipAlert = null;return false;}}
						});
					}else{
						//alert('失败,请重试');
					}
				}
				return inpmv(5741);
			},
			error:function(){
				alert('失败,请重试');
			}
		});
	}
	function comDocsAnimate(comDocsNum){

		var anmTipsH = 48;

		if(readerConfig.showStyle == 2){//1：竖版 2：横版
			anmTipsH = 26;
		}

		var imgUrl = 'http://221.122.117.73/index.jsp?file='+productId+'&width=40&pageno='+pageCur;

		var pointL = jQuery('#j_comdoc').offset().left - jQuery(obj).offset().left,
			pointT = jQuery('#j_comdoc').offset().top - jQuery(obj).offset().top + 5;

		if(jQuery(obj).children('.anmt-tips').length == 0){
			jQuery(obj).html(
				jQuery(obj).html()+'<div class="anmt-tips"></div'
			);
			jQuery(obj).find('.anmt-tips').css({
				'height' : anmTipsH,
				'background-image': 'url('+ imgUrl +')'
			});
		}

		jQuery(obj).children('.anmt-tips')
			.animate({top:60, left:'-50'},'slow')
			.animate({top:pointT, left:pointL, opacity:.5},'normal',"swing",function(){
				jQuery(this).remove();
				jQuery(obj).attr('title', '');
				jQuery(obj).removeClass('addcomdocs-on').addClass('addcomdocs-off').children('p').html('已加入');
				if(jQuery('#comdoc_num').length == 0){
					jQuery('.comdoc_btn_wrap').html(jQuery('.comdoc_btn_wrap').html()+'<em class="comdoc_num" id="comdoc_num"></em>');
				}
				if(jQuery('#comdoc_num').text() > 98){
					jQuery('#comdoc_num').text('...');
				}else if(jQuery('#comdoc_num').text() < 99){
					jQuery('#comdoc_num').text(comDocsNum);
				}
			}
		);
	}
	function comDocsUpNum(){
		var comDocsUpNum = new CreateDocinDialog({
			id:"comDocsUpNum",
			cls:"comDocsUpNum",
			title:" ",
			content:'<div style="padding:14px 36px 0 50px;"><p style="font-size:14px;font-weight:bold;line-height:28px;">抱歉，无法进行添加，组合文档最多为50页。建议您先在<a class="dialog_closed_by_link" href="/jsp_cn/productEnd/end_new_v1/doc_end_com_v1.jsp" target="_blank" title="我的组合" style="margin:0 3px;">我的组合</a>中进行数据的整理后，再进行新组合文档的添加。</p><p style="padding:28px 0 24px 79px;"><a class="dialog_closed_by_link verifyBtn" style="margin:0;" href="/jsp_cn/productEnd/end_new_v1/doc_end_com_v1.jsp" target="_blank" title="查看我的组合">查看我的组合</a></p></div>',
			button:2,
			callBack:{
				okBack:function(){
					comDocsUpNum = null;
					return true;
				},
				noBack:function(){
					comDocsUpNum = null;
					return false;
				}
			}
		});
	}
}