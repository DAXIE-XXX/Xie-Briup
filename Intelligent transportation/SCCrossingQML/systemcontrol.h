#ifndef SYSTEMCONTROL_H
#define SYSTEMCONTROL_H

#include <QObject>

class SystemControl : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int crossingId READ getCrossingId NOTIFY crossingIdChanged)
    Q_PROPERTY(int currentMode READ getMode NOTIFY modeChanged)
protected:
    explicit SystemControl(QObject *parent = nullptr);

public:
    enum LightMode{
        AutoMode = 0,
        HorMode = 1,
        VerMode = 2,
        NoMode = 3,
        NightMode = 4
    };
    static SystemControl *getObject();

    void system_init();
    void systen_start();

    Q_INVOKABLE void setMode(LightMode mode);
    Q_INVOKABLE int getMode() const { return mode; }
    Q_INVOKABLE int getCrossingId() const { return croId; }

signals:
    void modeChanged(int mode);
    void crossingIdChanged(int id);

protected slots:
    void crossingModeSetSlot(int id, int mode);
    void lightColorChangedslot();

protected:
    void timerEvent(QTimerEvent *e);

    static SystemControl *obj;
    LightMode mode = LightMode::AutoMode;
    int *autoModeHorBuf, *autoModeVerBuf;
    int *horModeHorBuf, *horModeVerBuf;
    int *verModeHorBuf, *verModeVerBuf;
    int *noModeHorBuf, *noModeVerBuf;
    int *nightModeHorBuf, *nightModeVerBuf;

    int *currentHorBuf, *currentVerBuf;
    int bufSize;
    int currentIndex;
    int croId = -1;
    int greenTime = 5;
};

#endif // SYSTEMCONTROL_H