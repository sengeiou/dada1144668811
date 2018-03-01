(function(){
	var config={
		domain:"http://t.ecmvs.com/taobao",
		adsvr_domain:"",
		is_open_replace:false,
		is_open_stat:false,
		is_show_detail:false,
		is_show_detail2:false,
		deviation_pixel:0
	};

	function rand(){
		var res=Math.random()*100000;
		return Math.floor(res);
	};
	function json2str(o,dep){
		if(typeof(JSON)!='undefined') return JSON.stringify(o,null,'\t');
		var space=function(c){
			var res="";
			for(var i=0;i<c;i++) res+='    ';
			return res;
		};
		var t=typeof(o);
		var res="";
		var first=true;
		var comma="";
		if(typeof(dep)=='undefined') dep=0;
		switch(t){
			case 'object':{
				if(!o) return 'null';
				if (Object.prototype.toString.apply(o) == '[object Array]'){
					res="[";
					for(var i=0;i<o.length;i++){
						if(!first) comma=",";
						else first=false;
						res+=(comma+"\r\n"+space(dep+1)+json2str(o[i],dep+1));
					}
					return res+"\r\n"+space(dep)+"]";
				}
				res="{";
				for(key in o){
					if(!first) comma=",";
					else first=false;
					res+=comma+"\r\n"+space(dep+1)+""+key+":"+json2str(o[key],dep+1);
				}
				res+="\r\n"+space(dep)+"}";
				return res;
			}
			case 'function':
			case 'null':
			case 'boolean':
			case 'number':{
				return o.toString();
			}
			case 'string':{
				return '"'+o.toString()+'"';
			}
			default:{
				return "'???[type:"+t+"]'";
			}
		}
	};
	function log_str(str){
		if(typeof(console)=='undefined') return;
		if(config.is_show_detail) console.log(str);
	};
	function log_json(json){
		var str=json2str(json);
		log_str(str);
	};
	function load_script(ansyc,src){
		if(ansyc){
			var n = document.createElement("script");
			n.type = "text/javascript";
			n.src = src;
			var h=document.getElementsByTagName('head')[0];
			h.appendChild(n);
		}
		else{
			document.write("<script src='"+src+"'></script>");
		}
	};
	function rpc_creator(){
		return {
			call: function (module, fun, args, callback) {
				jQueryDX.ajax({
				   type: "POST",
				   url: config.adsvr_domain+"/entry.php",
				   dataType:"jsonp",
				   jsonp:"jsonp_callback",
				   data: {
						module:module,
						method: fun,
						msg_id: 0,
						args:args
					},
				   success: function (data) {
						callback(data['result'],data);
				   }
				});
			}
		};
	};
	function advert_manager_creator(){
		var values=[];
		return {
			init:function(values_){
				values=values_;
				for(var i=0;i<values.length;i++){
					if(typeof(values[i].wrap)=='undefined') values[i].wrap="default";
				}
			},
			request_ad:function(w,h){
				var ads=[];
				for(var i=0;i<values.length;i++){
					var it=values[i];
					if(Math.abs(it.w-w)>config.deviation_pixel) continue;
					if(Math.abs(it.h-h)>config.deviation_pixel) continue;
					if(it.repeat<=0) continue;
					ads.push(it);
				}
				// alert(ads.length);
				var weights=0;
				for(var i=0;i<ads.length;i++){
					weights+=ads[i].weight;
				}
				// alert(weights);
				var position=rand()%weights;
				var cursor=0;
				for(var i=0;i<ads.length;i++){
					cursor+=ads[i].weight;
					if(cursor>position) {
						if(ads[i].src=='') return;
						else return ads[i];
					}
				}
			},
			make_url:function(it,from){
				if(typeof(from)=='undefined') from="";
				if(it.wrap=='nowrap') return it.src;
				var wrap_page=config.wrap_page[it.wrap];
				if(typeof(wrap_page)=='undefined') wrap_page=config.wrap_page["default"];
				var args=[
					"d="+it.id+"_"+from+"_"+it.w+"x"+it.h,
					"i="+it.id,
					"w="+it.w,
					"h="+it.h,
					"f="+from,
					"s="+encodeURIComponent(it.src)
				];
				return wrap_page+"?"+args.join("&");
			},
			make_frame:function(it,from){
				var src=this.make_url(it,from);
				return ('<iframe width="'+it.w+'" height="'+it.h+'" src="'+src+'" marginwidth="0" marginheight="0" scrolling="no" allowtransparency="true" align="center,center" frameborder="0"></iframe>');
			}
		};
	};
	function ad_stat_manager_creator(){
		var data_=[];
		var add_send_fn_=function (ad_stat){
			if(jQueryDX){
				add_send_fn_=function(){};
				jQueryDX(document).ready(function(){
					ad_stat.send();
				});
			}
		}
		return {
			add:function(w,h){
				for(var i=0;i<data_.length;i++){
					if(data_[i].w==w && data_[i].h==h){
						data_[i].c+=1;
						return;
					}
				}
				data_.push({w:w,h:h,c:1,rc:0});
				add_send_fn_(this);
			},
			add2:function(w,h){
				for(var i=0;i<data_.length;i++){
					if(data_[i].w==w && data_[i].h==h){
						data_[i].rc+=1;
						return;
					}
				}
				add_send_fn_(this);
			},
			send:function(){
				if(!config.is_open_stat) return;
				if(data_.length==0) return;
				var rpc=rpc_creator();
				var args={
					domain:location.host,
					ref:location.href,
					ads:data_
				};
				rpc.call('stat','stat_size',args,function(result,data){
					log_json(data);
					data_=[];
				});
			}
		};
	};
	if(typeof($gadproc) == 'undefined') $gadproc={};
	if(typeof($gadproc.init) == 'undefined'){
		$gadproc.init=function(){
			$gadproc.init=function(){};
			$gadproc.config=config;
			$gadproc.rand=rand;
			$gadproc.json2str=json2str;
			$gadproc.log_str=log_str;
			$gadproc.log_json=log_json;
			$gadproc.rpc_creator=rpc_creator;
			$gadproc.advert=advert_manager_creator();
			$gadproc.ad_stat=ad_stat_manager_creator();
			$gadproc.load_cfg=function(cfg){
				config=cfg;
				$gadproc.config=cfg;
				$gadproc.advert.init(cfg.data);
			};
			$gadproc.init_delay=function(){
			};
			load_script($gadproc.async,config.domain+"/jquery.min.js");
			load_script($gadproc.async,config.domain+"/config.js");
		}
	}
	$gadproc.init();
	if($gadproc.next_js) load_script($gadproc.async,config.domain+$gadproc.next_js);
})();
