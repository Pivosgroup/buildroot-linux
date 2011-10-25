#ifndef XMLTHREAD_H
#define XMLTHREAD_H

#include <QThread>

/*QT_BEGIN_NAMESPACE
class QFile;
class QHttp;
class QHttpResponseHeader;
class QSslError;
class QAuthenticator;
QT_END_NAMESPACE*/

class XMLThread : public QThread
{
    Q_OBJECT

public:
protected:
    XMLThread(QObject *parent = 0);
public:
    static XMLThread &Instance();
    ~XMLThread();
    void sleep(unsigned long);

protected:
    void run();

public slots:
private:

};

#endif // XMLTHREAD_H
