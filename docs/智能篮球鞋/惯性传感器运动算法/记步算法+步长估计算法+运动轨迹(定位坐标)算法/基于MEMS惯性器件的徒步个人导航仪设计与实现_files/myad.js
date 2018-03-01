// JavaScript Document
$(document).ready(function(){
	if(screen.width>1024)
	{
		$(".left-ad").show();
	}
	$(".left-ad .design1 .close").click(function()
	{
			$(".left-ad").slideUp("fast");
			$(".left-ad").slideDown("fast");
	});
	$(".left-ad .hint .rerun").click(function()
	{
			$(".left-ad").slideDown("fast");
			$(".left-ad").slideUp("fast");
	});
	$(".left-ad .hint .close").click(function()
	{
			$(".left-ad").slideUp("fast");
			$(".left-ad").slideUp("fast");
	});
});