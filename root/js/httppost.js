//����POST������ת��ָ��ҳ��
function httpPost(URL, PARAMS) {
    var temp = document.createElement("form");
    temp.method = "post";
    temp.style.display = "none";

    for (var x in PARAMS) {
        var opt = document.createElement("textarea");
        opt.name = x;
        opt.value = PARAMS[x];
        temp.appendChild(opt);
    }

    document.body.appendChild(temp);
    temp.submit();

    return temp;
}
