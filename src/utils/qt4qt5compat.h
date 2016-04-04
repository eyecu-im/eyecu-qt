#ifndef QT4QT5COMPAT
#define QT4QT5COMPAT

#if QT_VERSION < 0x050000
#define HTML_ESCAPE(value) Qt::escape(value)
#define HTML_ESCAPE_CHARS(value) Qt::escape(value)
#define URL_ENCODE(value) .setEncodedUrl(value)
#define SETRESIZEMODE setResizeMode
#define SETCLICABLE setClickable
#define URL_QUERY_ITEM_VALUE(URL,VALUE) URL.queryItemValue(VALUE)
#define URL_ADD_QUERY_ITEM(URL,KEY,VALUE) URL.addQueryItem(KEY,VALUE)
#define URL_SET_QUERY_ITEMS(URL,ITEMS) URL.setQueryItems(ITEMS)
#define URL_QUERY_ITEMS(URL) URL.queryItems()
#define URL_SET_QUERY_DELIMITERS(URL,VALUE,PAIR) URL.setQueryDelimiters(VALUE,PAIR)
#define TOASCII() toAscii()
#define MAYBE_JOIN
#else
#define HTML_ESCAPE(value) (value).toHtmlEscaped()
#define HTML_ESCAPE_CHARS(value) QString(value).toHtmlEscaped()
#define URL_ENCODE(value) = QUrl::fromEncoded(value)
#define SETRESIZEMODE setSectionResizeMode
#define SETCLICABLE setSectionsClickable
#define URL_QUERY_ITEM_VALUE(URL,KEY) QUrlQuery(URL).queryItemValue(KEY)
#define URL_ADD_QUERY_ITEM(URL,KEY,VALUE) QUrlQuery(URL).addQueryItem(KEY,VALUE)
#define URL_SET_QUERY_ITEMS(URL,ITEMS) QUrlQuery(URL).setQueryItems(ITEMS)
#define URL_QUERY_ITEMS(URL) QUrlQuery(URL).queryItems()
#define URL_SET_QUERY_DELIMITERS(URL,VALUE,PAIR) QUrlQuery(URL).setQueryDelimiters(VALUE,PAIR)
#define TOASCII() toLatin1()
#define MAYBE_JOIN .join("; ")
#endif

#endif // QT4QT5COMPAT

