import time

def url_to_get_request(url):
    parts = url.split(':')
    host = parts[0].replace('http://', '')
    port = parts[1] if len(parts) == 3 else '80'

    path = '/' + '/'.join(host.split('/')[1:])
    host = host.split('/')[0]

    extension = path.split('.')[-1]
    if extension == 'html':
        accept = 'text/html'
    elif extension == 'pdf':
        accept = 'application/pdf'
    elif extension == 'jpg':
        accept = 'image/jpeg'
    else:
        accept = 'text/*'

    modified_since = time.mktime(time.gmtime()) - 2 * 24 * 60 * 60
    modified_since = time.strftime("%a, %d %b %Y %H:%M:%S GMT", time.gmtime(modified_since))

    headers = f"""
GET {path} HTTP/1.1
Host: {host}
Connection: close
Date: {time.strftime("%a, %d %b %Y %H:%M:%S GMT", time.gmtime())}
Accept: {accept}
Accept-Language: en-us,en;q=0.9
If-Modified-Since: {modified_since}
    """

    return headers

url = "http://10.98.78.2/docs/a1.pdf:8080"
print(url_to_get_request(url))
