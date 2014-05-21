#ifndef CAMERAINTERPOLATION_H
#define CAMERAINTERPOLATION_H

#include "Utilities\XMFloatUtil.h"

#include <qobject.h>

class CameraInterpolation : public QObject
{
	Q_OBJECT
private:
	Vector3 m_StartPos, m_EndPos;
	float m_Timer, m_CurrentTimer;
	bool m_Active;
public:
	CameraInterpolation();

	void update(float p_dt);

	void createPath(Vector3 p_StartPos, Vector3 p_EndPos, float p_Time);

signals:
	void setCameraPositionSignal(Vector3);
};
#endif // CAMERAINTERPOLATION_H
