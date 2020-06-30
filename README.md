# HttpClient-C-
<hr><p><center><b>In English</b></center></p><hr>
<pre><b>INSTALLATION (EN):</b>
<i>-- Download the Http.h file and place it in the directory with the project, which is supposed to work with http requests.</i>
<i>-- In the project, which is supposed to work with http requests, you just need to exclude this file.</i></pre>
<br>
<pre style="line-height: 0"><center><b>EXAMPLE OF USE:</b></center><br><br>#include &lt;iostream&gt;<br>#include &lt;string&gt;<br>#include "Http.h"  <i>// We include the downloaded Http.h file</i><br><br>
using namespace std;<br><br>
int main(){<br>
    string datas = "Hello from C++.";<br>
    string head = "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:72.0) Gecko/20100101 Firefox/72.0\r\n";
    head += "Connection: Keep-Alive\r\n";
    head += "Cookie: PHPSESSID=r2t5uvjq435r4q7ib3vtdjq120\r\n";
    head += std::string("Content-Length: ") + std::to_string(datas.size());<br>
    Http http;
    http.setHeader(head); <i>// The title of the request to be transmitted. (Not necessary)</i>
    string answer = http.POST(L"http://mysite.com:3333/path_to_php_or_js_file_on_server", datas);<br>
    cout << answer << endl; <i>// We look that the server returned to us.</i><br>
    return 0;
}</pre>
<br>
<p><ul><li>The performance of sending and receiving data was verified both using the local server and using VPS.</li><li>The program for sending requests using the Http.h file was built using Microsoft Visual Studio Community 2019 Version 16.4.2.</li></ul></p>
<br><hr><p><center><b>In Russian</b></center></p><hr>
<pre><b>УСТАНОВКА (RU):</b>
<i>-- Скачать Http.h файл и поместить его в каталог с проектом, в котором предполагается работа с http запросами.</i>
<i>-- В проекте, в котором предполагается работа с http запросами, необходимо будет просто заинклюдить данный файл.</i></pre>
<br>
<pre style="line-height: 0"><center><b>ПРИМЕР ИСПОЛЬЗОВАНИЯ:</b></center><br><br>#include &lt;iostream&gt;<br>#include &lt;string&gt;<br>#include "Http.h"  <i>// Инклюдим скачанный Http.h файл</i><br><br>
using namespace std;<br><br>
int main(){<br>
    string datas = "Hello from C++.";<br>
    string head = "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:72.0) Gecko/20100101 Firefox/72.0\r\n";
    head += "Connection: Keep-Alive\r\n";
    head += "Cookie: PHPSESSID=r2t5uvjq435r4q7ib3vtdjq120\r\n";
    head += std::string("Content-Length: ") + std::to_string(datas.size());<br>
    Http http;
    http.setHeader(head); <i>// Заголовок запроса, который будем передавать. (НЕ обязательно)</i>
    string answer = http.POST(L"http://mysite.com:3333/path_to_php_or_js_file_on_server", datas);<br>
    cout << answer << endl; <i>// Смотрим, что вернул нам сервер.</i><br>
    return 0;
}</pre>
<br>
<p><ul><li>Работоспособность отправки и получения данных проверена как с использованием локального сервера, так и с использованием VPS.</li><li>Сборка программы по отправке запросов, с использованием Http.h файла, производилась с использованием Microsoft Visual Studio Community 2019 Version 16.4.2.</li></ul></p>


