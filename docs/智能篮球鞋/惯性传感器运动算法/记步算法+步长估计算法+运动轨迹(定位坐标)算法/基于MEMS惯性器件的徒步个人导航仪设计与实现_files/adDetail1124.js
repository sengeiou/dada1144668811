lastScrollY = 0;
var arr_id = new Array("left_gg", "right_gg", "right_80x80", "left_80x80");
function heartBeat(arr_id) {
    try {
        var diffY;
        if (document.documentElement && document.documentElement.scrollTop)
            diffY = document.documentElement.scrollTop;
        else if (document.body)
            diffY = document.body.scrollTop
        else
        { /*Netscape stuff*/ }

        percent = .1 * (diffY - lastScrollY);
        if (percent > 0) percent = Math.ceil(percent);
        else percent = Math.floor(percent);

        for (var i = 0; i < arr_id.length; i++) {
            if (document.getElementById(arr_id[i]) != null) {
                document.getElementById(arr_id[i]).style.top = parseInt(document.getElementById(arr_id[i]).style.top) + percent + "px";
            }
        }

        lastScrollY = lastScrollY + percent;
    }
    catch (qq) {

    }
}
window.setInterval("heartBeat(arr_id)", 1);

function closeBanner(div_id) {
    document.getElementById(div_id).style.display = 'none';
    return;
}
function addfavorite() {
    if (document.all) {
        window.external.addFavorite('http://www.cnki.com.cn', '知网空间-全球最大的数字图书馆！');
    }
    else if (window.sidebar) {
        window.sidebar.addPanel('知网空间-全球最大的数字图书馆！', 'http://www.cnki.com.cn', '知网空间-全球最大的数字图书馆！');
    }
}
function detailSearch() {
    var kw = document.getElementById('txt_word').value;
    if (kw == '输入关键词') {
        window.open('http://search.cnki.com.cn/search.aspx?q=' + escape('<xsl:value-of select="$QueryKey1"/><xsl:value-of select="$QueryKey2"/>'));
    }
    else if (kw == '') {
        window.open('http://search.cnki.com.cn/');
    }
    else {
        window.open('http://search.cnki.com.cn/search.aspx?q=' + escape(document.getElementById('txt_word').value));
    }
}

function YJSearch() {
    var kw = document.getElementById('txt_word').value;
    if (kw == '输入关键词') {
        window.open('http://yuanjian.cnki.com.cn/Search/Result?content=' + escape('<xsl:value-of select="$QueryKey1"/><xsl:value-of select="$QueryKey2"/>'));
    }
    else if (kw == '') {
        window.open('http://yuanjian.cnki.com.cn/');
    }
    else {
        window.open('http://yuanjian.cnki.com.cn/Search/Result?content=' + escape(document.getElementById('txt_word').value));
    }
}


function magazineSearch() {
    var kw = document.getElementById('txt_word').value;
    if (kw == '' || kw == '输入关键词') {
        window.open('http://Search.cnki.com.cn/SearchMagazine.aspx');
    }
    else {
        window.open('http://Search.cnki.com.cn/SearchMagazine.aspx?q=' + escape(document.getElementById('txt_word').value));
    }
}
function change_color() {
    if (location.href.toLowerCase().indexOf("www.cnki.com.cn") > 0) {
        document.getElementById("header_cjfd").className = "navon";
    }
    else if (location.href.toLowerCase().indexOf("cdmd") > 0) {
        document.getElementById("header_cdmd").className = "navon";
    }
    else if (location.href.toLowerCase().indexOf("cpfd") > 0) {
        document.getElementById("header_cpfd").className = "navon";
    }
    else {
        document.getElementById("header_cjfd").className = "navon";
    }
}
function closetopad() {
    document.getElementById("jpylstopad").style.display = "none";
}


function downArticle(curObj) {
    try {       
        var d_fn = curObj.getAttribute("data-fn");
        var d_dbcode = curObj.getAttribute("data-dbcode");
        var d_year = curObj.getAttribute("data-year");
        var d_dflag = curObj.getAttribute("data-dflag");
        var url="http://search.cnki.net/down/default.aspx?filename=" + d_fn + "&dbcode=" + d_dbcode + "&year=" + d_year + "&dflag=" + d_dflag;
        //alert(d_fn+d_dbcode+d_year+d_dflag);
        try {
            if (/MSIE\s*(\d+\.\d+);/.test(navigator.userAgent) || /MSIE(\d+\.\d+);/.test(navigator.userAgent) || navigator.userAgent.indexOf('MSIE')>=0||navigator.userAgent.indexOf('msie')>=0) {
                var referLink = document.createElement('a');
                referLink.href = url;
                document.body.appendChild(referLink);
                referLink.click();
            } else {
                window.location.href = url;
            } 
        } catch (e) {
            window.location.href = url;
        }
        
    } catch (e) {
        alert(e);
    }
}