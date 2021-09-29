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

//var a =`a,b=map(int,input().split())
//print(a+b)
//`

//post_judge(a, "1000", "python3")
