#ifndef _DEBUGER_H
#define _DEBUGER_H

#include <QDialog>
#include <QLineEdit>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QKeyEvent>
#include <QTimer>

//#include <stdint.h>

#define DASMROW 25
#define DMPSIZE 16

struct DasmRow {
	ushort adr;
	QString bytes;
	QString dasm;
};

struct CatchPoint {
	bool active;
	ushort adr;
	ushort sp;
};

class DebugWin : public QDialog {
	Q_OBJECT
	public:
		DebugWin(QWidget*);
		CatchPoint cpoint;
		bool active;
		void reject();
		void start();
		void stop();
	private:
		QLineEdit *ledit;
		QGridLayout *rglay,*asmlay,*dmplay,*raylay,*vglay;
		QLabel *dbgscrn,*tlab;
		QLabel* logLabel;
		uchar rowincol[16];
		ushort upadr,adr,dmpadr;
		uchar curcol, currow;
		QList<DasmRow> fdasm;
		bool fillall();
		void fillregz();
		bool filldasm();
		void filldump();
		void fillrays();
		void fillvg();
		void filltick();
		void showedit(QLabel*,QString);
		ushort getprevadr(ushort);
		DasmRow getdisasm();
		bool tmpb;
		unsigned int t;
		void doStep();
		bool logging;
		QTimer logTimer;
		void startLog();
		void stopLog();
	private slots:
		void logStep();
	protected:
		void keyPressEvent(QKeyEvent*);
};

void dbgInit(QWidget*);
void dbgShow();
bool dbgIsActive();

#endif
