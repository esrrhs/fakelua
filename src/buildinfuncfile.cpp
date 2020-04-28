#include "buildinfuncfile.h"
#include "fake.h"
#include "fake-inc.h"

// fopen
void buildin_fopen(fake *fk, interpreter *inter) {
    BIF_CHECK_ARG_NUM(2);

    const char *mod = fkpspopcstrptr(fk);
    const char *filename = fkpspopcstrptr(fk);
    FILE *f = fopen(filename, mod);
    if (f) {
        fkpspush<bool>(fk, true);
        fkpspush<FILE *>(fk, f);
    } else {
        fkpspush<bool>(fk, false);
        fkpspush<FILE *>(fk, f);
    }
}

// fclose
void buildin_fclose(fake *fk, interpreter *inter) {
    BIF_CHECK_ARG_NUM(1);

    FILE *f = fkpspop<FILE *>(fk);
    if (f) {
        fclose(f);
    }
    fkpspush<int>(fk, 1);
}

// freadall
void buildin_freadall(fake *fk, interpreter *inter) {
    BIF_CHECK_ARG_NUM(1);

    FILE *f = fkpspop<FILE *>(fk);
    if (f) {
        fseek(f, 0, SEEK_END);
        size_t filesize = ftell(f);
        fseek(f, 0, SEEK_SET);

        char *buffer = (char *) safe_fkmalloc(fk, filesize + 1, emt_tmp);
        buffer[filesize] = 0;
        if (fread(buffer, filesize, 1, f) == 1) {
            fkpspush<char *>(fk, buffer);
            safe_fkfree(fk, buffer);
            return;
        }
        safe_fkfree(fk, buffer);
    }
    const char *ret = "";
    fkpspush<const char *>(fk, ret);
}

// fwriteall
void buildin_fwriteall(fake *fk, interpreter *inter) {
    BIF_CHECK_ARG_NUM(2);

    const char *data = fkpspopcstrptr(fk);
    FILE *f = fkpspop<FILE *>(fk);
    if (f && data) {
        size_t datasize = strlen(data);

        if (fwrite(data, datasize, 1, f) == 1) {
            fkpspush<bool>(fk, true);
            return;
        }
    }
    fkpspush<bool>(fk, false);
}

void buildinfuncfile::openfilefunc() {
    m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("fopen"), buildin_fopen);
    m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("fclose"), buildin_fclose);
    m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("freadall"), buildin_freadall);
    m_fk->fm.add_buildin_func(m_fk->sh.allocsysstr("fwriteall"), buildin_fwriteall);
}

