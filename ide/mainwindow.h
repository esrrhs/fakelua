#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCore>
#include <QtGui>
#include <QFileDialog>
#include <QMessageBox>
#include <QColor>
#include <QInputDialog>
#include <QLayout>
#include <QPlainTextEdit>
#include <QTextCursor>
#include <QShortcut>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <qvector.h>
#include <qlistwidget.h>

#include "../include/fakescript.h"

namespace Ui
{
    class MainWindow;
}

struct Breakpoint
{
    QString file;
    int line;
};

typedef QVector<Breakpoint> BreakpointList;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void Output(QString str);
    void SetError(const char * file, int lineno, const char * func);

private slots:
    void on_actionCopy_triggered();

    void on_actionOpen_triggered();

    void on_actionNew_triggered();

    void on_actionRedo_triggered();

    void on_actionPaste_triggered();

    void on_actionCut_triggered();

    void on_actionUndo_triggered();

    void on_actionSave_triggered();

    void on_actionSave_as_triggered();

    void on_actionQuit_triggered();

    void on_actionFIND_triggered();

    void on_btnfindprev_clicked();

    void on_btnfindnext_clicked();

    void on_actionReplace_triggered();

    void on_btnreplaceprev_clicked();

    void on_btnreplacenext_clicked();

    void on_actionFont_triggered();

private slots:
    void highlightCurrentLine();

    void on_txtedit_textChanged();

    void on_actionGoto_Line_triggered();

    void on_btngo_clicked();

    void on_MainWindow_destroyed();

    void on_MainWindow_destroyed(QObject *arg1);

    void on_actionBreakpoint_triggered();

    void on_actionBuild_triggered();

    void on_actionRun_triggered();

    void on_actionRun_Args_triggered();

    void on_actionStep_triggered();

    void on_txtrunarg_editingFinished();

    void on_txtrunarg_returnPressed();

    void on_txtfind_returnPressed();

    void on_txtgo_returnPressed();

    void on_actionRun_With_Debug_triggered();

    void on_actionStep_In_triggered();

    void on_txtedit_cursorPositionChanged();

    void on_callstackview_clicked(const QModelIndex &index);

    void on_routineview_clicked(const QModelIndex &index);

    void on_variant_value_editingFinished();

    void on_memview_itemClicked(QListWidgetItem *item);

private:
    void UpdateStatusBar();
    void CheckSave();
    bool Build();
    bool Check_trigger_breakpoint();
    void Openfile(QString file);
    void UpdateDebugView();
    void StopDebug(int ret);

protected:
    virtual void closeEvent(QCloseEvent *);

private:
    Ui::MainWindow * m_ui;
    QString m_file_name;
    QString m_saved;

    fake * m_fk;
    const char * m_argv[100];
    int m_errorline;
    BreakpointList m_BreakpointList;
    bool m_isdebug;
    bool m_isrun;
    int m_runline;
    int m_frame;
    int m_rid;
};

QT_BEGIN_NAMESPACE
class QTextDocument;
QT_END_NAMESPACE

class QSyntaxHighlighterFake : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    QSyntaxHighlighterFake(QTextDocument *parent = 0);

protected:
    void highlightBlock(const QString &text);

private:
    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat keywordFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;
};

#endif // MAINWINDOW_H
