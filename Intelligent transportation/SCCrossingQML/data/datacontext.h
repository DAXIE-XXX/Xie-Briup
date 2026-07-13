#ifndef DATACONTEXT_H
#define DATACONTEXT_H

#include <QObject>
#include <QJsonArray>
class LightInfo;
class DataContext : public QObject
{
    Q_OBJECT
protected:
    explicit DataContext(QObject *parent = nullptr);

public:
    static DataContext *getObject();
    Q_INVOKABLE void setLightColor(int hc, int vc);
    Q_INVOKABLE void setLightHColor(int hc);
    Q_INVOKABLE void setLightVColor(int vc);
    Q_INVOKABLE QJsonArray getLightList();
    void setLeftLightInfo(int id, bool show, int roadId);
    void setRightLightInfo(int id, bool show, int roadId);
    void setUpLightInfo(int id, bool show, int roadId);
    void setDownLightInfo(int id, bool show, int roadId);

signals:
    void lightColorChanged();

protected:
    static DataContext *obj;
    QVector<LightInfo *> ll;
};

class LightInfo {
    friend class DataContext;
public:
    LightInfo(int x, int y, int w, int h, int dir);
protected:
    int x,y,w,h;
    int color = 0;  //0-g 1-y 2-r
    int dir = -1;   //0-left 1-right 2-up 3-down
    int id = -1;
    bool show = false;
    int roadId = -1;
};

#endif // DATACONTEXT_H
