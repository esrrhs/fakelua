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

void fk_error_func(fake * fk, int eno, const char * str)
{
    static char tmp[1000 * 1000];

    sprintf(tmp, "fake error[%d] in file(%s:%d) func(%s) : %s\ncall stack :\n%s\n", eno, fkgetcurfile(fk), fkgetcurline(fk), fkgetcurfunc(fk), str, fkgetcurcallstack(fk));
    g_MainWindow->Output(tmp);
}

void fk_print_func(fake * fk, const char * str)
{
    g_MainWindow->Output(str);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow)
{
    g_MainWindow = this;

    m_ui->setupUi(this);
    m_ui->dockWidget->setVisible(false);
    m_ui->dockWidget_2->setVisible(false);
    m_ui->dockWidget_3->setVisible(false);
    m_ui->dockWidget_4->setVisible(false);
    QVBoxLayout *findLayout = new QVBoxLayout;
    findLayout->addWidget(m_ui->txtfind);
    findLayout->addWidget(m_ui->btnfindnext);
    findLayout->addWidget(m_ui->btnfindprev);
    m_ui->dockWidget->setLayout(findLayout);

    m_file_name.clear();
    m_ui->txtedit->setPlainText("");
    m_ui->txtedit->setVisible(true);
    m_saved.clear();

    connect(m_ui->txtedit, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

    highlightCurrentLine();

    m_ui->txtedit->setFont(QFont(tr("宋体"), 20, QFont::Normal));
    m_ui->txtedit->setLineWrapMode(QPlainTextEdit::NoWrap);

    new QSyntaxHighlighterFake(m_ui->txtedit->document());

    memset(m_argv, 0, sizeof(m_argv));

    m_fk = newfake();
    fkopenalllib(m_fk);
    fkseterrorfunc(m_fk, fk_error_func);
    fksetprintfunc(m_fk, fk_print_func);
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

    if (!m_ui->txtedit->isReadOnly())
    {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::yellow).lighter(160);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = m_ui->txtedit->textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
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
    UpdateStatusBar();
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
}

void MainWindow::on_actionBuild_triggered()
{
    Build();
}

void MainWindow::on_actionRun_triggered()
{
    if (Build())
    {
        int ret = fkrun<int>(m_fk, "main");
        if (fkerror(m_fk))
        {
            return;
        }
        Output("Exit " + QString::number(ret, 10));
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

}

bool MainWindow::Build()
{
    m_ui->output->clear();
    on_actionSave_triggered();
    Output("Start Build " + m_file_name);

    QTextCodec * code = QTextCodec::codecForName("GB2312");
    std::string tmp = code->fromUnicode(m_file_name).data();

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
