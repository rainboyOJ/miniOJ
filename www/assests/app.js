// 通过fetch 提交评测的数据
// code 代码
// pid 评测的题目编号
// lang 语言
function post_judge(code,pid,lang){
  return fetch('/judge', { // 在URL中写上传递的参数
    method: 'POST',
    headers: new Headers({
      "pid":pid,
      "lang":lang,
      "Content-Type":"text/html"
    }),
    body: code
  }).then( res=> res.json() ) // 返回一个Promise，可以解析成JSON
}

function base64Decode(input) {
  rv = window.atob(input);
  rv = escape(rv);
  rv = decodeURIComponent(rv);
  return rv;
}

//var a =`a,b=map(int,input().split())
//print(a+b)
//`

//post_judge(a, "1000", "python3")

// =============== result.html js
//拿到数据
function base64TOjson(str_base64){
  //console.log(base64Decode(str_base64))
  return JSON.parse(base64Decode(str_base64));
}

var error_mean = {
  "0":         "SUCCESS",
  "-1":        "INVALID_CONFIG",
  "-2":        "FORK_FAILED",
  "-3":        "PTHREAD_FAILED",
  "-4":        "WAIT_FAILED",
  "-5":        "ROOT_REQUIRED",
  "-6":        "LOAD_SECCOMP_FAILED",
  "-7":        "SETRLIMIT_FAILED",
  "-8":        "DUP2_FAILED",
  "-9":        "SETUID_FAILED",
  "-10":       "EXECVE_FAILED",
  "-11":       "SPJ_ERROR",
 }

var result_mean = {
  "-1": ["W","WRONG_ANSWER"],
  "0":  ["A","ACCEPT"],
  "1":  ["T","CPU_TIME_LIMIT_EXCEEDED"],
  "2":  ["T","REAL_TIME_LIMIT_EXCEEDED"],
  "3":  ["M","MEMORY_LIMIT_EXCEEDED"],
  "4":  ["R","RUNTIME_ERROR"],
  "5":  ["S","SYSTEM_ERROR"]
};

function short_res_class(chr){
  let _class = ""
  switch(chr){
    case 'W':  _class='one-short-W'; break; 
    case 'A':  _class='one-short-A'; break;
    case 'T':  _class='one-short-T'; break;
    case 'M':  _class='one-short-M'; break;
    case 'R':  _class='one-short-R'; break;
    case 'S':  _class='one-short-S'; break;
  }
  return _class;
}

function get_judge_result(id){
  return fetch(`/result/${id}/json`).then( res=> res.json() ) // 返回一个Promise，可以解析成JSON
}
function set_content_by_id(id,content){
  document.getElementById(id).innerText = content
}


function sleep (time) {
  return new Promise((resolve) => setTimeout(resolve, time));
}

function set_hidden(id){
  document.getElementById(id).style = "display:none";
}

//对result页面进行修改
async function set_result(){
  //1.得到url 中的id
  let id_reg = /result\/(\d+)\/?/
  let id="";
  if( id_reg.test(window.location.pathname) )
    id = window.location.pathname.match(id_reg)[1]

  let res = await get_judge_result(id)
  //console.log(res)

  //只有在WAITING 和JUDGING 的时候会不停的轮询
  set_content_by_id("status",res.status);
  set_content_by_id("msg",res.msg);
  set_content_by_id("judge-id", id)
  set_content_by_id("pid", res.pid)
  document.getElementById("pid").href = `/problem/${res.pid}`

  //status WAITING, PROBLEM_ERROR, PROBLEM_DATA_NOT_EXISTS, JUDGING, COMPILE_ERROR, END
  if( res.status == 'WAITING'){
    await sleep(1000); //等一秒
    //continue
  }
  set_hidden('loading');
    
  //一般设置
 if(res.status == 'END') {
    let results_html = ""
    let short_res = ""
    let idx = 0;
    for( let r of res.results){
      let [s,l] = result_mean[r.result+'']
      results_html += `<tr><td>${++idx}</td>
      <td>${ (r.memory /1024 / 1024).toFixed(4)} mb</td>
      <td>${r.cpu_time} ms</td>
      <td>
       <span class="${short_res_class(s)}">
        ${l}
       </span>
        </td></tr>`
      short_res += `<span class="${ short_res_class(s) }">${s}</span>`
    }
    document.getElementById("results").innerHTML      = results_html;
    document.getElementById("short-result").innerHTML = short_res;
    set_hidden('msg-father');
  }


  return;

}
// =============== result.html js === end
