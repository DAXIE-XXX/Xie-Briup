#ifndef SYSTEMCONTROL_H
#define SYSTEMCONTROL_H

#include <QObject>
#include <QVector>

class SystemControl : public QObject
{
    Q_OBJECT
protected:
    explicit SystemControl(QObject *parent = nullptr);

public:
    static SystemControl *getObject();
    void autoStart();

protected slots:
    void lightColorChangedSlot(int id, int roadId, int color);

protected:
    static SystemControl *obj;
};

#endif // SYSTEMCONTROL_H