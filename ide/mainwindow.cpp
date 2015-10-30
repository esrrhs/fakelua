#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <qfontdialog.h>

#ifdef _DEBUG
#pragma comment(lib, "../bin/fakescript_d.lib")
#else
#pragma comment(lib, "../bin/fakescript.lib")
#endif

MainWindow * g_MainWindow = 0;
extern QApplication * g_ide;

void fk_error_func(fake * fk, int eno, const char * file, int lineno, const char * func, const char * str)
{
    static char tmp[1000 * 1000];

    g_MainWindow->SetError(file, lineno, func);

    sprintf(tmp, "fake error[%d] in file(%s:%d) func(%s) : %s\ncall stack :\n%s\n", eno, file, lineno, func, str, fkgetcurcallstack(fk));
    g_MainWindow->Output(tmp);
}

void fk_print_func(fake * fk, const char * str)
{
    g_MainWindow->Output(str);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_errorline(-1),
    m_isdebug(false),
    m_isrun(false),
    m_runline(-1),
    m_frame(0),
    m_rid(0)
{
    g_MainWindow = this;

    m_ui->setupUi(this);
    m_ui->dockWidget->setVisible(false);
    m_ui->dockWidget_2->setVisible(false);
    m_ui->dockWidget_3->setVisible(false);
    m_ui->dockWidget_4->setVisible(false);

    m_file_name.clear();
    m_ui->txtedit->setPlainText("");
    m_ui->txtedit->setVisible(true);
    m_saved.clear();

    m_ui->txtedit->setFont(QFont(tr("宋体"), 20, QFont::Normal));
    m_ui->txtedit->setLineWrapMode(QPlainTextEdit::NoWrap);
    highlightCurrentLine();

    m_ui->callstackview->setFont(QFont(tr("宋体"), 20, QFont::Normal));
    m_ui->callstackview->setSpacing(5);

    m_ui->routineview->setFont(QFont(tr("宋体"), 20, QFont::Normal));
    m_ui->routineview->setSpacing(5);

    m_ui->byteview->setFont(QFont(tr("宋体"), 10, QFont::Normal));
    m_ui->byteview->setLineWrapMode(QTextEdit::NoWrap);

    m_ui->memview->setFont(QFont(tr("宋体"), 10, QFont::Normal));
    m_ui->memview->setSpacing(5);

    new QSyntaxHighlighterFake(m_ui->txtedit->document());

    memset(m_argv, 0, sizeof(m_argv));

    m_fk = newfake();
    fkopenalllib(m_fk);
    fkseterrorfunc(m_fk, fk_error_func);
    fksetprintfunc(m_fk, fk_print_func);

    QFileInfo fi("sample.fk");
    if (fi.exists())
    {
        Openfile("sample.fk");
        UpdateStatusBar();
    }

    m_ui->callstackview->hide();
    m_ui->byteview->hide();
    m_ui->memview->hide();
    m_ui->routineview->hide();
    m_ui->horizontalLayout_2->removeItem(m_ui->verticalLayout);
}

MainWindow::~MainWindow()
{
    delete m_ui;
}

void MainWindow::Output(QString str)
{
    m_ui->output->insertPlainText(str + "\n");
    QTextCursor cursor =  m_ui->output->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_ui->output->setTextCursor(cursor);
    g_ide->processEvents();
}

void MainWindow::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    QTextEdit::ExtraSelection selection;

    // cur
    {
        QColor lineColor = QColor(Qt::yellow).lighter(160);
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = m_ui->txtedit->textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    // error
    if (m_errorline != -1)
    {
        QColor lineColor = QColor(Qt::red).lighter(160);
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);

        QTextCursor tc = m_ui->txtedit->textCursor();
        int position = m_ui->txtedit->document()->findBlockByNumber(m_errorline - 1).position();
        tc.setPosition(position, QTextCursor::MoveAnchor);
        selection.cursor = tc;
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    foreach (const Breakpoint & bp, m_BreakpointList)
    {
        if (bp.file == m_file_name)
        {
            QColor lineColor = QColor(Qt::cyan).lighter(160);
            selection.format.setBackground(lineColor);
            selection.format.setProperty(QTextFormat::FullWidthSelection, true);

            QTextCursor tc = m_ui->txtedit->textCursor();
            int position = m_ui->txtedit->document()->findBlockByNumber(bp.line - 1).position();
            tc.setPosition(position, QTextCursor::MoveAnchor);
            selection.cursor = tc;
            selection.cursor.clearSelection();
            extraSelections.append(selection);
        }
    }

    if (m_isdebug)
    {
        if (m_runline != -1)
        {
            QColor lineColor = QColor(Qt::green).lighter(160);
            selection.format.setBackground(lineColor);
            selection.format.setProperty(QTextFormat::FullWidthSelection, true);

            QTextCursor tc = m_ui->txtedit->textCursor();
            int position = m_ui->txtedit->document()->findBlockByNumber(m_runline - 1).position();
            tc.setPosition(position, QTextCursor::MoveAnchor);
            selection.cursor = tc;
            selection.cursor.clearSelection();
            extraSelections.append(selection);
        }
    }

    m_ui->txtedit->setExtraSelections(extraSelections);

    UpdateStatusBar();
}

void MainWindow::on_actionNew_triggered()
{
    CheckSave();
    m_file_name.clear();
    m_ui->txtedit->setPlainText("");
    m_saved.clear();
    UpdateStatusBar();
}

void MainWindow::on_actionOpen_triggered()
{
    CheckSave();
    m_saved.clear();
    QString file = QFileDialog::getOpenFileName(this, "Open a file", "", "Fake (*.fk)");

    if(!file.isEmpty())
    {
        Openfile(file);
    }
    UpdateStatusBar();
}

void MainWindow::Openfile(QString file)
{
    QFile sFile(file);
    if(sFile.open(QFile::ReadOnly | QFile::Text))
    {
        m_file_name = file;
        QTextStream in(&sFile);
        in.setCodec("UTF-8");
        QString text = in.readAll();
        sFile.close();

        m_ui->txtedit->setPlainText(text);
        m_saved = text;
    }
}

void MainWindow::on_actionCopy_triggered()
{
    m_ui->txtedit->copy();
}

void MainWindow::on_actionPaste_triggered()
{
    m_ui->txtedit->paste();
}

void MainWindow::on_actionCut_triggered()
{
    m_ui->txtedit->cut();
}

void MainWindow::on_actionUndo_triggered()
{
    m_ui->txtedit->undo();
}

void MainWindow::on_actionRedo_triggered()
{
    m_ui->txtedit->redo();
}

void MainWindow::on_actionSave_triggered()
{
    if(m_file_name.isEmpty())
    {
        on_actionSave_as_triggered();
    }
    else
    {
        QFile sFile(m_file_name);
        if(sFile.open(QFile::WriteOnly | QFile::Text))
        {
            QTextStream out(&sFile);
            out.setCodec("UTF-8");

            out << m_ui->txtedit->toPlainText();
            m_saved = m_ui->txtedit->toPlainText();

            sFile.flush();
            sFile.close();
        }
    }
    UpdateStatusBar();
}


void MainWindow::on_actionSave_as_triggered()
{
    QString file = QFileDialog::getSaveFileName(this, "Enter File", "", "Fake (*.fk)");
    if(!file.isEmpty())
    {
        m_file_name = file;
        on_actionSave_triggered();
    }
}

void MainWindow::on_actionQuit_triggered()
{
    CheckSave();
    exit(0);
}

void MainWindow::on_actionFIND_triggered()
{
    m_ui->dockWidget_3->setVisible(false);
    m_ui->dockWidget_4->setVisible(false);
    m_ui->dockWidget_2->setVisible(false);
    m_ui->dockWidget->setVisible(true);
    m_ui->txtfind->setFocus();
}

void MainWindow::on_btnfindprev_clicked()
{
    QString str = m_ui->txtfind->text();
    bool ok = m_ui->txtedit->find(str,QTextDocument::FindBackward);
}

void MainWindow::on_btnfindnext_clicked()
{
    QTextCursor cursor;
    QString str = m_ui->txtfind->text();
    bool ok = m_ui->txtedit->find(str);
}

void MainWindow::on_actionReplace_triggered()
{
    m_ui->dockWidget_3->setVisible(false);
    m_ui->dockWidget_4->setVisible(false);
    m_ui->dockWidget->setVisible(false);
    m_ui->dockWidget_2->setVisible(true);
    m_ui->txtreplace1->setFocus();
}

void MainWindow::on_btnreplaceprev_clicked()
{
    QTextCursor cursor;
    QString str = m_ui->txtreplace1->text();
    bool ok = m_ui->txtedit->find(str,QTextDocument::FindBackward);
    if(ok)
    {
        cursor=m_ui->txtedit->textCursor();
        cursor.insertText(m_ui->txtreplace2->text());
    }
}

void MainWindow::on_btnreplacenext_clicked()
{
    QTextCursor cursor;
    QString str = m_ui->txtreplace1->text();
    bool ok = m_ui->txtedit->find(str);
    if(ok)
    {
        cursor=m_ui->txtedit->textCursor();
        cursor.insertText(m_ui->txtreplace2->text());
    }
}

void MainWindow::on_actionFont_triggered()
{
    bool b = false;
    QFont font = QFontDialog::getFont(&b, QFont(tr("宋体"), 20, QFont::Normal), this, tr("自定义文字属性0"));
    if (b)
    {
        m_ui->txtedit->setFont(font);
    }
}

void MainWindow::UpdateStatusBar()
{
    int col = m_ui->txtedit->textCursor().positionInBlock();
    QTextLayout * pLayout = m_ui->txtedit->textCursor().block().layout();
    int nCurpos = m_ui->txtedit->textCursor().position() - m_ui->txtedit->textCursor().block().position();
    int line = pLayout->lineForTextPosition(nCurpos).lineNumber() + m_ui->txtedit->textCursor().block().firstLineNumber() + 1;

    QString title = "fake ide ";
    title += QFileInfo(m_file_name).fileName();
    if (m_saved != m_ui->txtedit->toPlainText())
    {
        title += " *";
    }
    setWindowTitle(title);

    char buff[1024];
    sprintf(buff, "line %d, col %d, total %d", line, col, m_ui->txtedit->toPlainText().size());
    QString msg = buff;
    m_ui->statusBar->showMessage(msg);
}

void MainWindow::on_txtedit_textChanged()
{
    UpdateStatusBar();
}

void MainWindow::CheckSave()
{
    if(m_saved != m_ui->txtedit->toPlainText())
    {
        QMessageBox msgBox;
        msgBox.setText("The document has been modified.");
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();
        switch (ret)
        {
           case QMessageBox::Save:
               on_actionSave_triggered();
               break;
           case QMessageBox::Discard:
               break;
        }
    }
}

void MainWindow::on_actionGoto_Line_triggered()
{
    m_ui->dockWidget_4->setVisible(false);
    m_ui->dockWidget_2->setVisible(false);
    m_ui->dockWidget->setVisible(false);
    m_ui->dockWidget_3->setVisible(true);
    m_ui->txtgo->setFocus();
}

void MainWindow::on_btngo_clicked()
{
    int line = m_ui->txtgo->text().toInt();

    QTextCursor tc = m_ui->txtedit->textCursor();
    int position = m_ui->txtedit->document()->findBlockByNumber(line - 1).position();
    tc.setPosition(position,QTextCursor::MoveAnchor);
    m_ui->txtedit->setTextCursor( tc );
    m_ui->dockWidget_3->setVisible(false);
    m_ui->txtedit->setFocus();
}

void MainWindow::on_MainWindow_destroyed()
{
    CheckSave();
}

void MainWindow::on_MainWindow_destroyed(QObject *arg1)
{
    CheckSave();
}

void MainWindow::closeEvent(QCloseEvent * event)
{
    CheckSave();
    event->accept();
}

QSyntaxHighlighterFake::QSyntaxHighlighterFake(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    keywordFormat.setForeground(Qt::blue);
    keywordFormat.setFontWeight(QFont::Bold);
    const char ** fkkw = fkgetkeyword();
    int i = 0;
    QString tmp = fkkw[i];
    while (tmp != "")
    {
        QString pattern = "\\b" + tmp + "\\b";
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
        i++;
        tmp = fkkw[i];
    }

    functionFormat.setForeground(Qt::red);
    rule.pattern = QRegExp("\\b[A-Za-z0-9_\.]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    quotationFormat.setForeground(Qt::gray);
    rule.pattern = QRegExp("\".*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    singleLineCommentFormat.setForeground(Qt::green);
    rule.pattern = QRegExp("--[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);
}

void QSyntaxHighlighterFake::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules)
    {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0)
        {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
    setCurrentBlockState(0);
}

void MainWindow::on_actionBreakpoint_triggered()
{
    QTextLayout * pLayout = m_ui->txtedit->textCursor().block().layout();
    int nCurpos = m_ui->txtedit->textCursor().position() - m_ui->txtedit->textCursor().block().position();
    int line = pLayout->lineForTextPosition(nCurpos).lineNumber() + m_ui->txtedit->textCursor().block().firstLineNumber() + 1;

    for (BreakpointList::iterator it = m_BreakpointList.begin(); it != m_BreakpointList.end(); it++)
    {
        Breakpoint & bp = *it;
        if (bp.file == m_file_name && bp.line == line)
        {
            m_BreakpointList.erase(it);
            highlightCurrentLine();
            return;
        }
    }

    Breakpoint bp;
    bp.file = m_file_name;
    bp.line = line;
    m_BreakpointList.push_back(bp);

    highlightCurrentLine();
}

void MainWindow::on_actionBuild_triggered()
{
    Build();
}

void MainWindow::on_actionRun_triggered()
{    
    if (m_isdebug || m_isrun)
    {
        return;
    }

    if (Build())
    {
        m_isrun = true;
        fkclosestepmod(m_fk);
        int ret = fkrun<int>(m_fk, "main");
        if (fkerror(m_fk))
        {
            return;
        }
        Output("Exit " + QString::number(ret, 10));
        m_isrun = false;
    }
}

void MainWindow::on_actionRun_Args_triggered()
{
    m_ui->dockWidget_3->setVisible(false);
    m_ui->dockWidget_2->setVisible(false);
    m_ui->dockWidget->setVisible(false);
    m_ui->dockWidget_4->setVisible(true);
    m_ui->txtrunarg->setFocus();
}

void MainWindow::on_actionStep_triggered()
{
    if (!m_isdebug || m_isrun)
    {
        return;
    }

    m_frame = 0;
    m_rid = fkgetcurroutineid(m_fk);

    int ret = 0;
    bool isend = false;

    QString lastfile = fkgetcurfile(m_fk);
    int lastline = fkgetcurline(m_fk);
    int laststacklength = fkgetcurcallstacklength(m_fk);
    int lastrid = m_rid;
    QString curfile = fkgetcurfile(m_fk);
    int curline = fkgetcurline(m_fk);
    int curstacklength = fkgetcurcallstacklength(m_fk);
    int currid = m_rid;
    while (currid == lastrid && ((curstacklength > laststacklength) || (lastfile == curfile && lastline == curline)))
    {
        ret = fkresume<int>(m_fk, isend);
        if (isend)
        {
            StopDebug(ret);
            return;
        }
        curfile = fkgetcurfile(m_fk);
        curline = fkgetcurline(m_fk);
        curstacklength = fkgetcurcallstacklength(m_fk);
        currid = fkgetcurroutineid(m_fk);
        if (Check_trigger_breakpoint())
        {
            break;
        }
    }

    m_rid = fkgetcurroutineid(m_fk);
    UpdateDebugView();
}

bool MainWindow::Build()
{
    if (m_isdebug || m_isrun)
    {
        return false;
    }

    m_errorline = -1;
    highlightCurrentLine();
    m_ui->output->clear();
    on_actionSave_triggered();
    Output("Start Build " + m_file_name);

    QTextCodec * code = QTextCodec::codecForName("GB2312");
    std::string tmp = code->fromUnicode(m_file_name).data();

    fkclear(m_fk);
    fkparse(m_fk, tmp.c_str());

    if (fkerror(m_fk))
    {
        Output("Build " + m_file_name + " Fail");
        return false;
    }
    else
    {
        Output("Build " + m_file_name + " OK");
        return true;
    }
}

void MainWindow::on_txtrunarg_editingFinished()
{
    on_txtrunarg_returnPressed();
}

void MainWindow::on_txtrunarg_returnPressed()
{
    QString args = m_ui->txtrunarg->text();
    QStringList argslist = args.split(" ");

    int i = 0;
    foreach (const QString & arg, argslist)
    {
        std::string stmp = arg.toStdString();
        const char * tmp = stmp.c_str();
        free((void*)m_argv[i]);
        m_argv[i] = (const char *)calloc(strlen(tmp) + 1, 1);
        memcpy((void*)m_argv[i], tmp, strlen(tmp));
        i++;
    }

    fksetargv(m_fk, argslist.size(), m_argv);
}

void MainWindow::on_txtfind_returnPressed()
{
    on_btnfindnext_clicked();
}

void MainWindow::on_txtgo_returnPressed()
{
    on_btngo_clicked();
}

void MainWindow::on_actionRun_With_Debug_triggered()
{
    int ret = 0;
    bool isend = false;

    m_frame = 0;
    m_rid = fkgetcurroutineid(m_fk);

    if (m_isdebug || m_isrun)
    {
        while (1)
        {
            ret = fkresume<int>(m_fk, isend);
            if (isend)
            {
                StopDebug(ret);
                return;
            }
            if (Check_trigger_breakpoint())
            {
                break;
            }
        }

        m_rid = fkgetcurroutineid(m_fk);
        UpdateDebugView();

        return;
    }

    if (Build())
    {
        m_ui->callstackview->show();
        m_ui->byteview->show();
        m_ui->memview->show();
        m_ui->routineview->show();
        m_ui->horizontalLayout_2->insertLayout(1, m_ui->verticalLayout, 2);
        m_isdebug = true;
        bool firstrun = true;
        fkopenstepmod(m_fk);
        fkrun<int>(m_fk, "main");
        while (1)
        {
            if (firstrun)
            {
                if (Check_trigger_breakpoint())
                {
                    firstrun = false;
                    break;
                }
            }
            firstrun = false;

            ret = fkresume<int>(m_fk, isend);
            if (isend)
            {
                StopDebug(ret);
                return;
            }
            if (Check_trigger_breakpoint())
            {
                break;
            }
        }
    }
    m_rid = fkgetcurroutineid(m_fk);
    UpdateDebugView();
}

bool MainWindow::Check_trigger_breakpoint()
{
    QString curfile = fkgetcurfile(m_fk);
    int line = fkgetcurline(m_fk);
    foreach (const Breakpoint & bp, m_BreakpointList)
    {
        QTextCodec * code = QTextCodec::codecForName("GB2312");
        QString tmp = code->fromUnicode(bp.file).data();
        if (bp.line == line && tmp == curfile)
        {
            return true;
        }
    }

    return false;
}

void MainWindow::SetError(const char * file, int lineno, const char * func)
{
    m_errorline = lineno;
    highlightCurrentLine();

    QTextCursor tc = m_ui->txtedit->textCursor();
    int position = m_ui->txtedit->document()->findBlockByNumber(lineno - 1).position();
    tc.setPosition(position,QTextCursor::MoveAnchor);
    m_ui->txtedit->setTextCursor( tc );
    m_ui->txtedit->setFocus();
}

void MainWindow::on_actionStep_In_triggered()
{
    if (!m_isdebug || m_isrun)
    {
        return;
    }

    m_frame = 0;
    m_rid = fkgetcurroutineid(m_fk);

    int ret = 0;
    bool isend = false;

    QString lastfile = fkgetcurfile(m_fk);
    int lastline = fkgetcurline(m_fk);
    int lastrid = m_rid;
    QString curfile = fkgetcurfile(m_fk);
    int curline = fkgetcurline(m_fk);
    int curtrid = m_rid;
    while (lastrid == curtrid && (lastfile == curfile && lastline == curline))
    {
        ret = fkresume<int>(m_fk, isend);
        if (isend)
        {
            StopDebug(ret);
            return;
        }
        curfile = fkgetcurfile(m_fk);
        curline = fkgetcurline(m_fk);
        if (Check_trigger_breakpoint())
        {
            break;
        }
    }

    m_rid = fkgetcurroutineid(m_fk);
    UpdateDebugView();
}

void MainWindow::UpdateDebugView()
{
    if (m_isdebug)
    {
        QString curfile = fkgetcurfilebyroutinebyframe(m_fk, m_rid, m_frame);
        if (curfile != m_file_name)
        {
            Openfile(curfile);
        }
        m_runline = fkgetcurlinebyroutinebyframe(m_fk, m_rid, m_frame);
    }

    highlightCurrentLine();

    if (m_isdebug && m_runline != -1)
    {
        QTextCursor tc = m_ui->txtedit->textCursor();
        int position = m_ui->txtedit->document()->findBlockByNumber(m_runline - 1).position();
        tc.setPosition(position,QTextCursor::MoveAnchor);
        m_ui->txtedit->setTextCursor( tc );
        m_ui->txtedit->setFocus();
    }

    // stack
    if (m_isdebug)
    {
        m_ui->callstackview->clear();

        int length = fkgetcurcallstacklength(m_fk);
        for (int i = 0; i < length; i++)
        {
            QString str = fkgetcurcallstackbyroutinebyframe(m_fk, m_rid, i);
            str.remove('\n');

            QListWidgetItem * tmp = new QListWidgetItem(m_ui->callstackview);
            tmp->setText(str);
            if (i == m_frame)
            {
                tmp->setIcon(QIcon(":/image/logo.ico"));
                tmp->setBackgroundColor(QColor(Qt::green));
            }
            m_ui->callstackview->addItem(tmp);
        }
    }

    // byte
    if (m_isdebug)
    {
        const char * func = fkgetcurfuncbyroutinebyframe(m_fk, m_rid, m_frame);
        int pos = fkgetcurbytecodeposbyroutine(m_fk, m_rid);
        QString str = fkdumpfunc(m_fk, func, pos);
        m_ui->byteview->setPlainText(str);
    }

    // routine
    if (m_isdebug)
    {
        m_ui->routineview->clear();

        int num = fkgetcurroutinenum(m_fk);
        for (int i = 0; i < num; i++)
        {
            QString str = fkgetcurroutinebyindex(m_fk, i);
            str.remove('\n');

            int currid = fkgetroutineidbyindex(m_fk, i);
            if (currid == fkgetcurroutineid(m_fk))
            {
                str = "[ Run ]" + str;
            }
            else
            {
                str = "[Sleep]" + str;
            }

            QListWidgetItem * tmp = new QListWidgetItem(m_ui->routineview);
            tmp->setText(str);
            if (currid == m_rid)
            {
                tmp->setIcon(QIcon(":/image/logo.ico"));
                tmp->setBackgroundColor(QColor(Qt::green));
            }
            m_ui->routineview->addItem(tmp);
        }
    }

    // mem
    if (m_isdebug)
    {
        m_ui->memview->clear();

        const char * func = fkgetcurfuncbyroutinebyframe(m_fk, m_rid, m_frame);
        int num = fkgetfuncvariantnum(m_fk, func);

        for (int i = 0; i < num; i++)
        {
            const char * name = fkgetfuncvariantname(m_fk, func, i);
            int line = fkgetfuncvariantline(m_fk, func, i);
            const char * value = fkgetcurvaiantbyroutinebyframe(m_fk, m_rid, m_frame, name, line);

            {
                QListWidgetItem * tmp = new QListWidgetItem(m_ui->memview);
                m_ui->memview->addItem(tmp);

                QWidget * l = new QWidget();
                QHBoxLayout * ll = new QHBoxLayout(l);

                QLabel * a = new QLabel(l);
                a->setText(name);

                QLineEdit * b = new QLineEdit(l);
                b->setText(value);
                connect(b, SIGNAL(editingFinished()), this, SLOT(on_variant_value_editingFinished()));

                QLabel * c = new QLabel(l);
                QString cstr = "(" + QString::number(line, 10) + ")";
                c->setText(cstr);

                ll->addWidget(a);
                ll->addWidget(c);
                ll->addWidget(b);
                l->show();

                m_ui->memview->setItemWidget(tmp, l);

                tmp->setSizeHint(QSize(l->rect().width(), l->rect().height()));
            }
        }
    }
}

void MainWindow::on_txtedit_cursorPositionChanged()
{
    highlightCurrentLine();
}

void MainWindow::on_callstackview_clicked(const QModelIndex &index)
{
    m_frame = index.row();

    UpdateDebugView();
}

void MainWindow::on_routineview_clicked(const QModelIndex &index)
{
    m_rid = fkgetroutineidbyindex(m_fk, index.row());

    UpdateDebugView();
}

void MainWindow::StopDebug(int ret)
{
    Output("Exit " + QString::number(ret, 10));
    m_isdebug = false;
    m_runline = -1;
    UpdateDebugView();

    m_ui->callstackview->hide();
    m_ui->byteview->hide();
    m_ui->memview->hide();
    m_ui->routineview->hide();
    m_ui->horizontalLayout_2->removeItem(m_ui->verticalLayout);
}

void MainWindow::on_variant_value_editingFinished()
{
    QObject * s = this->sender();
    QLineEdit * c = (QLineEdit*)s;
    QString value = c->text();

    QWidget * l = (QWidget *)(c->parent());

    QList<QLabel*> lists = l->findChildren<QLabel*>();
    QLabel * a = lists[0];
    QLabel * b = lists[1];

    QString name = a->text();
    QString tmp = b->text();
    tmp.remove("(");
    tmp.remove(")");
    int line = tmp.toInt();

    std::string namestd = name.toStdString();
    std::string valuestd = value.toStdString();

    fksetcurvaiantbyroutinebyframe(m_fk, m_rid, m_frame, namestd.c_str(), valuestd.c_str(), line);
}

void MainWindow::on_memview_itemClicked(QListWidgetItem *item)
{
    QWidget * l = m_ui->memview->itemWidget(item);

    QList<QLabel*> lists = l->findChildren<QLabel*>();

    QLabel * b = lists[1];

    QString tmp = b->text();
    tmp.remove("(");
    tmp.remove(")");
    int line = tmp.toInt();

    QTextCursor tc = m_ui->txtedit->textCursor();
    int position = m_ui->txtedit->document()->findBlockByNumber(line - 1).position();
    tc.setPosition(position,QTextCursor::MoveAnchor);
    m_ui->txtedit->setTextCursor( tc );
    m_ui->txtedit->setFocus();
}
