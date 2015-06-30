#ifndef QT4QT5COMPAT
#define QT4QT5COMPAT

#if QT_VERSION < 0x050000
#define HTML_ESCAPE(value) Qt::escape(value)
#define HTML_ESCAPE_CHARS(value) Qt::escape(value)
#define URL_ENCODE(value) .setEncodedUrl(value)
#define SETRESIZEMODE setResizeMode
#define SETCLICABLE setClickable
#define TOASCII() toAscii()
#define MAYBE_JOIN
#else
#define HTML_ESCAPE(value) (value).toHtmlEscaped()
#define HTML_ESCAPE_CHARS(value) QString(value).toHtmlEscaped()
#define URL_ENCODE(value) = QUrl::fromEncoded(value)
#define SETRESIZEMODE setSectionResizeMode
#define SETCLICABLE setSectionsClickable
#define TOASCII() toLatin1()
#define MAYBE_JOIN .join("; ")
#endif

#endif // QT4QT5COMPAT

