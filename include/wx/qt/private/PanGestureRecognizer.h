#include <QGestureRecognizer>



class PanGestureRecognizer : public QGestureRecognizer
{
private:
   static const int MINIMUM_DISTANCE = 10;

   typedef QGestureRecognizer parent;

   bool IsValidMove(int dx, int dy);

   virtual QGesture* create(QObject* pTarget);

   virtual QGestureRecognizer::Result recognize(QGesture* pGesture, QObject *pWatched, QEvent *pEvent);

   void reset (QGesture *pGesture);

   uint m_started;
   QPointF m_startPoint;
   QPointF m_lastPoint;
};
