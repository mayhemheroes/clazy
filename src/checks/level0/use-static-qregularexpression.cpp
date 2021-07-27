/*
  This file is part of the clazy static checker.

  Copyright (C) 2021 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Author: Waqar Ahmed <waqar.ahmed@kdab.com>

  Copyright (C) 2021 Waqar Ahmed <waqar.17a@gmail.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "use-static-qregularexpression.h"
#include "Utils.h"
#include "HierarchyUtils.h"
#include "QtUtils.h"
#include "TypeUtils.h"

#include <clang/AST/AST.h>
#include <clang/AST/ExprCXX.h>

using namespace clang;
using namespace std;

UseStaticQRegularExpression::UseStaticQRegularExpression(const std::string &name, ClazyContext *context)
    : CheckBase(name, context)
{
}

static bool isArgNonStaticLocalVar(Expr *arg0)
{
    auto declRefExpr = dyn_cast_or_null<DeclRefExpr>(clazy::getFirstChild(arg0));
    if (!declRefExpr) {
        return false;
    }

    auto varDecl = dyn_cast_or_null<VarDecl>(declRefExpr->getDecl());
    if (!varDecl) {
        return false;
    }

    return varDecl->isLocalVarDecl() && !varDecl->isStaticLocal();
}

static bool isArgTemporaryObj(Expr *arg0)
{
    return dyn_cast_or_null<MaterializeTemporaryExpr>(arg0);
}

static bool isQStringOrQStringListMethod(CXXMethodDecl *methodDecl)
{
    return clazy::isOfClass(methodDecl, "QString") || clazy::isOfClass(methodDecl, "QStringList");
}

static bool firstArgIsQRegularExpression(CXXMethodDecl* methodDecl, const LangOptions& lo)
{
    return clazy::simpleArgTypeName(methodDecl, 0, lo) == "QRegularExpression";
}

void UseStaticQRegularExpression::VisitStmt(clang::Stmt *stmt)
{
    if (!stmt) {
        return;
    }

    auto method = dyn_cast_or_null<CXXMemberCallExpr>(stmt);
    if (!method) {
        return;
    }

    if (method->getNumArgs() == 0) {
        return;
    }

    auto methodDecl = method->getMethodDecl();
    if (!isQStringOrQStringListMethod(methodDecl)) {
        return;
    }
    if (!firstArgIsQRegularExpression(methodDecl, lo())) {
        return;
    }

    Expr* arg0 = method->getArg(0);
    if (!arg0) {
        return;
    }

    if (isArgTemporaryObj(arg0)) {
        emitWarning(clazy::getLocStart(arg0), "Don't create temporary QRegularExpression objects. Use a static QRegularExpression object instead");
        return;
    }

    if (isArgNonStaticLocalVar(arg0)) {
        emitWarning(clazy::getLocStart(arg0), "Don't create temporary QRegularExpression objects. Use a static QRegularExpression object instead");
        return;
    }
}