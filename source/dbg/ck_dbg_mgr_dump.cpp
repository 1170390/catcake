/*
    Copyright (c) 2007-2009 Takashi Kitao
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright
    `  notice, this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright
    `  notice, this list of conditions and the following disclaimer in the
    `  documentation and/or other materials provided with the distribution.

    3. The name of the author may not be used to endorse or promote products
    `  derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
    OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
    IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
    NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
    THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "ck_dbg_all.h"

#include "ck_sys_all.h"
#include "ck_task_all.h"
#include "ck_res_all.h"
#include "ck_conf_all.h"
#include "ck_util_all.h"


void ckDbgMgr::dumpVector(const ckVec& vec, const char* name)
{
    if (!m_instance)
    {
        return;
    }

    if (name)
    {
        trace("%s = ", name);
    }

    trace("(%.3f, %.3f, %.3f)\n", vec.x, vec.y, vec.z);
}


void ckDbgMgr::dumpMatrix(const ckMat& mat, const char* name)
{
    if (!m_instance)
    {
        return;
    }

    if (name)
    {
        trace("%s = ", name);
    }

    trace("((%.3f, %.3f, %.3f), ", mat.x_axis.x, mat.x_axis.y, mat.x_axis.z);
    trace("(%.3f, %.3f, %.3f), ", mat.y_axis.x, mat.y_axis.y, mat.y_axis.z);
    trace("(%.3f, %.3f, %.3f), ", mat.z_axis.x, mat.z_axis.y, mat.z_axis.z);
    trace("(%.3f, %.3f, %.3f))\n", mat.trans.x, mat.trans.y, mat.trans.z);
}


static bool isEqual(const char* str1, const char* str2)
{
    for ( ; ; str1++, str2++)
    {
        if (*str1 != *str2)
        {
            return false;
        }

        if (*str1 == '\0')
        {
            return true;
        }
    }
}


void ckDbgMgr::dumpMemory()
{
    if (!m_instance)
    {
        return;
    }

    trace("` DumpMemory `\n");
    trace("+--------------------+-----+--------+\n");
    trace("|        NAME        |BLOCK|  SIZE  |\n");
    trace("+--------------------+-----+--------+\n");

    for (const void* mem1 = ckMemMgr::getFirstMemoryBlockN(); mem1; mem1 = ckMemMgr::getNextMemoryBlockN(mem1))
    {
        const char* name = ckMemMgr::getMemoryBlockName(mem1);

        bool is_continue = false;

        for (const void* mem2 = ckMemMgr::getFirstMemoryBlockN(); mem2 != mem1; mem2 = ckMemMgr::getNextMemoryBlockN(mem2))
        {
            if (isEqual(name, ckMemMgr::getMemoryBlockName(mem2)))
            {
                is_continue = true;
                break;
            }
        }

        if (is_continue)
        {
            continue;
        }

        u32 count = 1;
        u32 size = ckMemMgr::getMemoryBlockSize(mem1);

        for (const void* mem2 = ckMemMgr::getNextMemoryBlockN(mem1); mem2; mem2 = ckMemMgr::getNextMemoryBlockN(mem2))
        {
            if (isEqual(name, ckMemMgr::getMemoryBlockName(mem2)))
            {
                count++;
                size += ckMemMgr::getMemoryBlockSize(mem2);
            }
        }

        trace("|%-20s|%5d|%8d|\n", ckUtil::getBasename(name), count, size);
    }

    trace("+--------------------+-----+--------+\n");
    trace("|TOTAL SIZE  %18d byte|\n", ckMemMgr::getCurUsedMemorySize());
    trace("|MAX SIZE    %18d byte|\n", ckMemMgr::getMaxUsedMemorySize());
    trace("+-----------------------------------+\n");
}


void ckDbgMgr::dumpTask()
{
    if (!m_instance)
    {
        return;
    }

    trace("` DumpTask `\n");
    trace("+--------------------+-----+------+\n");
    trace("|        NAME        |ORDER| TIME |\n");
    trace("+--------------------+-----+------+\n");

    for (u32 order = ckTask::ORDER_MINUS_8_FOR_SYSTEM; order <= ckTask::ORDER_PLUS_8_FOR_SYSTEM; order++)
    {
        for (ckTask* task = ckTaskMgr::getFirstTaskN(static_cast<ckTask::TaskOrder>(order)); task; task = task->getNextAllN())
        {
            static char buf1[8];

            if (task->hasOrder())
            {
                ckSysMgr::sprintf(buf1, 8, "%5d", task->getOrder() - 8);
            }
            else if (task->getParentN() == task->getPrevAllN())
            {
                ckSysMgr::sprintf(buf1, 8, "CHILD");
            }
            else
            {
                ckSysMgr::sprintf(buf1, 8, "child");
            }

            static char buf2[8];

            if (task->isEnabled())
            {
                ckSysMgr::sprintf(buf2, 8, "%6d", task->getExecuteUsecTime());
            }
            else
            {
                ckSysMgr::sprintf(buf2, 8, "PAUSED");
            }

            trace("|%-20s|%5s|%6s|\n", ckUtil::getBasename(task->getName()), buf1, buf2);
        }
    }

    trace("|%-20s|%5d|%6d|\n", "RENDER", 9, ckTaskMgr::getRenderUsecTime());

    trace("+--------------------+-----+------+\n");
    trace("|TOTAL TIME   %15d usec|\n", ckTaskMgr::getExecuteUsecTime());
    trace("+---------------------------------+\n");
}


void ckDbgMgr::dumpResource()
{
    if (!m_instance)
    {
        return;
    }

    trace("` DumpResource `\n");
    trace("+--------+---+--------+----+\n");
    trace("|   ID   |EXT|  SIZE  |AUTO|\n");
    trace("+--------+---+--------+----+\n");

    for (const ckRes* res = ckResMgr::getFirstResourceN(); res; res = ckResMgr::getNextResourceN(res->getID()))
    {
        trace("|%08X|%3s|%8d|%4s|\n", res->getID().getValue(), res->getExtension().getString(), //
            res->getDataSize(), res->isAutoFree() ? "ON" : "OFF");
    }

    trace("+--------+---+--------+----+\n");
    trace("TYPE = { ");

    for (const ckStr<char, 3>* str = ckResMgr::getFirstTypeN(); str; str = ckResMgr::getNextTypeN(*str))
    {
        if (str != ckResMgr::getFirstTypeN())
        {
            trace(", ");
        }

        trace("\"%s\"", str->getString());
    }

    trace(" }\n");
}


void ckDbgMgr::dumpConfig()
{
    if (!m_instance)
    {
        return;
    }

    trace("` DumpConfig `\n");
    trace("+--------+-------+-----+\n");
    trace("|   ID   | STATE |ERROR|\n");
    trace("+--------+-------+-----+\n");

    for (ckConf* conf = ckConfMgr::getFirstConfigN(); conf; conf = conf->getNextN())
    {
        trace("|%08X|%7s|%5d|\n", conf->getID().getValue(), conf->isValid() ? "VALID" : "INVALID", conf->isValid() ? 0 : conf->getErrorLineNo());
    }

    trace("+--------+-------+-----+\n");
}


void ckDbgMgr::dumpScreen()
{
    if (!m_instance)
    {
        return;
    }

    trace("` DumpScreen `\n");
    trace("+--------+---------+---------+----+\n");
    trace("|   ID   |   POS   |   SIZE  |FLAG|\n");
    trace("+--------+---------+---------+----+\n");

    for (ckScr* scr = ckDrawMgr::getFirstScreenN(); scr; scr = scr->getNextN())
    {
        trace("|%08X|%4d,%4d|%4dx%4d|%c%c%c%c|\n", scr->getID().getValue(), //
            scr->getLeftInFramebuffer(), scr->getTopInFramebuffer(), scr->getWidthInFramebuffer(), scr->getHeightInFramebuffer(), //
            scr->isEnabled() ? 'E' : '_', scr->isClearColor() ? 'C' : '_', scr->isClearDepth() ? 'D' : '_', scr->isPerspective() ? 'P' : '_');
    }

    trace("+--------+---------+---------+----+\n");
}


void ckDbgMgr::dumpTexture()
{
    if (!m_instance)
    {
        return;
    }

    static const char* s_format_name[] =
    {
        "RGB", "RGBA", "ALPHA", "PNG_RGB", "PNG_RGBA", "PNG_ALPHA"
    };

    static const char* s_mode_name[] =
    {
        "READ_ONLY", "READ_WRITE", "FRAMEBUFFER", "VOLATILE"
    };

    trace("` DumpTexture `\n");
    trace("+--------+---------+---------+-----------+\n");
    trace("|   ID   |   SIZE  |  FORMAT |    MODE   |\n");
    trace("+--------+---------+---------+-----------+\n");

    for (ckTex* tex = ckDrawMgr::getFirstTextureN(); tex; tex = tex->getNextN())
    {
        trace("|%08X|%4dx%4d|%9s|%11s|\n", tex->getID().getValue(), tex->getWidth(), tex->getHeight(), //
            s_format_name[tex->getFormat()], s_mode_name[tex->getMode()]);
    }

    trace("+--------+---------+---------+-----------+\n");
}


void ckDbgMgr::dumpShader()
{
    if (!m_instance)
    {
        return;
    }

    trace("` DumpShader `\n");
    trace("+--------+-------------------+\n");
    trace("| SHADER: %-11s        |\n", ckDrawMgr::isShaderAvailable() ? "AVAILABLE" : "UNAVAILABLE");
    trace("+--------+---+---+---+-------+\n");
    trace("|   ID   |UNI|ATT|TEX| STATE |\n");
    trace("+--------+---+---+---+-------+\n");

    for (ckShd* shd = ckDrawMgr::getFirstShaderN(); shd; shd = shd->getNextN())
    {
        trace("|%08X|%3d|%3d|%3d|%7s|\n", shd->getID().getValue(), shd->getUniformNum(), shd->getAttribNum(), //
            shd->getTextureNum(), shd->isValid() ? "VALID" : "INVALID");
    }

    trace("+--------+---+---+---+-------+\n");
}
