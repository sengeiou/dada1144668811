(function(){
	var $g=$gadproc;
	function stat(ad_inf){
		$g.ad_stat.add(ad_inf.stdwidth,ad_inf.stdheight);
	};
	function debug(ad_inf){
		var log_data={
			f:"tb",
			w:ad_inf.stdwidth,
			h:ad_inf.stdheight,
			pid:ad_inf.pid
		};
		var param=$g.json2str(log_data,null,'');
		var src=$g.config.domain+"/show_param.html?data="+encodeURIComponent(param);
		var frag='<iframe width="'+ad_inf.stdwidth+'" height="'+ad_inf.stdheight+'" src="'+src+'" marginwidth="0" marginheight="0" frameborder="0"></iframe>';

		if($g.config.is_show_detail) $g.log_json(log_data);
		if($g.config.is_show_detail2) return frag;
		return false;
	};
	function allown_replace(ad_inf){
		if(!$g.config.is_open_replace) return false;
		var tb_cfg=$g.config.tb;
		for(var i in tb_cfg.pbid){
			if(tb_cfg.pbid[i]==ad_inf.pid) return false;
		}
		return true;
	};
	function ad_s(ad_inf){
		if(typeof(ad_inf.stdwidth)=='undefined') ad_inf.stdwidth=parseInt(ad_inf.width);
		if(typeof(ad_inf.stdheight)=='undefined') ad_inf.stdheight=parseInt(ad_inf.height);
		stat(ad_inf);
		var debug_info=debug(ad_inf);
		if(debug_info) return debug_info;
		if(!allown_replace(ad_inf)) return false;
		var ad=$g.advert.request_ad(ad_inf.stdwidth,ad_inf.stdheight);
		if(typeof(ad)=='undefined') return false;
		ad.repeat--;
		return $g.advert.make_frame(ad,'tb');
	}
	if(typeof($gadproc.ad_s)=='undefined'){
		$g.ad_s=ad_s;
	};
	$g.init_delay();
	$g.init_tb();
})();
