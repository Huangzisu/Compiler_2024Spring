#include "ast2llvm.h"
#include <vector>
#include <unordered_map>
#include <string>
#include <cassert>
#include <list>

using namespace std;
using namespace LLVMIR;

static unordered_map<string,FuncType> funcReturnMap;
static unordered_map<string,StructInfo> structInfoMap;
static unordered_map<string,Name_name*> globalVarMap;
static unordered_map<string,Temp_temp*> localVarMap;
static list<L_stm*> emit_irs;

LLVMIR::L_prog* ast2llvm(aA_program p)
{
    auto defs = ast2llvmProg_first(p);
    auto funcs = ast2llvmProg_second(p);
    vector<L_func*> funcs_block;
    for(const auto &f : funcs)
    {
        funcs_block.push_back(ast2llvmFuncBlock(f));
    }
    for(auto &f : funcs_block)
    {
        ast2llvm_moveAlloca(f);
    }
    return new L_prog(defs,funcs_block);
}

int ast2llvmRightVal_first(aA_rightVal r)
{
    if(r == nullptr)
    {
        return 0;
    }
    switch (r->kind)
    {
    case A_arithExprValKind:
    {
        return ast2llvmArithExpr_first(r->u.arithExpr);
        break;
    }
    case A_boolExprValKind:
    {
        return ast2llvmBoolExpr_first(r->u.boolExpr);
        break;
    }
    default:
        break;
    }
    return 0;
}

int ast2llvmBoolExpr_first(aA_boolExpr b)
{
    switch (b->kind)
    {
    case A_boolBiOpExprKind:
    {
        return ast2llvmBoolBiOpExpr_first(b->u.boolBiOpExpr);
        break;
    }
    case A_boolUnitKind:
    {
        return ast2llvmBoolUnit_first(b->u.boolUnit);
        break;
    }
    default:
         break;
    }
    return 0;
}

int ast2llvmBoolBiOpExpr_first(aA_boolBiOpExpr b)
{
    int l = ast2llvmBoolExpr_first(b->left);
    int r = ast2llvmBoolExpr_first(b->right);
    if(b->op == A_and)
    {
        return l && r;
    }
    else
    {
        return l || r;
    }
}

int ast2llvmBoolUOpExpr_first(aA_boolUOpExpr b)
{
    if(b->op == A_not)
    {
        return !ast2llvmBoolUnit_first(b->cond);
    }
    return 0;
}

int ast2llvmBoolUnit_first(aA_boolUnit b)
{
    switch (b->kind)
    {
    case A_comOpExprKind:
    {
        return ast2llvmComOpExpr_first(b->u.comExpr);
        break;
    }
    case A_boolExprKind:
    {
        return ast2llvmBoolExpr_first(b->u.boolExpr);
        break;
    }
    case A_boolUOpExprKind:
    {
        return ast2llvmBoolUOpExpr_first(b->u.boolUOpExpr);
        break;
    }
    default:
        break;
    }
    return 0;
}

int ast2llvmComOpExpr_first(aA_comExpr c)
{
    auto l = ast2llvmExprUnit_first(c->left);
    auto r = ast2llvmExprUnit_first(c->right);
    switch (c->op)
    {
    case A_lt:
    {
        return l < r;
        break;
    }
    case A_le:
    {
        return l <= r;
        break;
    }
    case A_gt:
    {
        return l > r;
        break;
    }
    case A_ge:
    {
        return l >= r;
        break;
    }
    case A_eq:
    {
        return l == r;
        break;
    }
    case A_ne:
    {
        return l != r;
        break;
    }
    default:
        break;
    }
    return 0;
}

int ast2llvmArithBiOpExpr_first(aA_arithBiOpExpr a)
{
    auto l= ast2llvmArithExpr_first(a->left);
    auto r = ast2llvmArithExpr_first(a->right);
    switch (a->op)
    {
    case A_add:
    {
        return l + r;
        break;
    }
    case A_sub:
    {
        return l - r;
        break;
    }
    case A_mul:
    {
        return l * r;
        break;
    }
    case A_div:
    {
        return l / r;
        break;
    }
    default:
        break;
    }
    return 0;
}

int ast2llvmArithUExpr_first(aA_arithUExpr a)
{
    if(a->op == A_neg)
    {
        return -ast2llvmExprUnit_first(a->expr);
    }
    return 0;
}

int ast2llvmArithExpr_first(aA_arithExpr a)
{
    switch (a->kind)
    {
    case A_arithBiOpExprKind:
    {
        return ast2llvmArithBiOpExpr_first(a->u.arithBiOpExpr);
        break;
    }
    case A_exprUnitKind:
    {
        return ast2llvmExprUnit_first(a->u.exprUnit);
        break;
    }
    default:
        assert(0);
        break;
    }
    return 0;
}

int ast2llvmExprUnit_first(aA_exprUnit e)
{
    if(e->kind == A_numExprKind)
    {
        return e->u.num;
    }
    else if(e->kind == A_arithExprKind)
    {
        return ast2llvmArithExpr_first(e->u.arithExpr);
    }
    else if(e->kind == A_arithUExprKind)
    {
        return ast2llvmArithUExpr_first(e->u.arithUExpr);
    }
    else
    {
        assert(0);
    }
    return 0;
}

std::vector<LLVMIR::L_def*> ast2llvmProg_first(aA_program p)
{
    vector<L_def*> defs;
    defs.push_back(L_Funcdecl("getch",vector<TempDef>(),FuncType(ReturnType::INT_TYPE)));
    defs.push_back(L_Funcdecl("getint",vector<TempDef>(),FuncType(ReturnType::INT_TYPE)));
    defs.push_back(L_Funcdecl("putch",vector<TempDef>{TempDef(TempType::INT_TEMP)},FuncType(ReturnType::VOID_TYPE)));
    defs.push_back(L_Funcdecl("putint",vector<TempDef>{TempDef(TempType::INT_TEMP)},FuncType(ReturnType::VOID_TYPE)));
    defs.push_back(L_Funcdecl("putarray",vector<TempDef>{TempDef(TempType::INT_TEMP),TempDef(TempType::INT_PTR,-1)},FuncType(ReturnType::VOID_TYPE)));
    defs.push_back(L_Funcdecl("_sysy_starttime",vector<TempDef>{TempDef(TempType::INT_TEMP)},FuncType(ReturnType::VOID_TYPE)));
    defs.push_back(L_Funcdecl("_sysy_stoptime",vector<TempDef>{TempDef(TempType::INT_TEMP)},FuncType(ReturnType::VOID_TYPE)));
    funcReturnMap.emplace("getch", FuncType(ReturnType::INT_TYPE));
    funcReturnMap.emplace("getint", FuncType(ReturnType::INT_TYPE));
    funcReturnMap.emplace("putch", FuncType(ReturnType::VOID_TYPE));
    funcReturnMap.emplace("putint", FuncType(ReturnType::VOID_TYPE));
    funcReturnMap.emplace("putarray", FuncType(ReturnType::VOID_TYPE));
    funcReturnMap.emplace("_sysy_starttime", FuncType(ReturnType::VOID_TYPE));
    funcReturnMap.emplace("_sysy_stoptime", FuncType(ReturnType::VOID_TYPE));
    for(const auto &v : p->programElements)
    {
        switch (v->kind)
        {
        case A_programNullStmtKind:
        {
            break;
        }
        case A_programVarDeclStmtKind:
        {
            if(v->u.varDeclStmt->kind == A_varDeclKind)
            {
                if(v->u.varDeclStmt->u.varDecl->kind == A_varDeclScalarKind)
                {
                    if(v->u.varDeclStmt->u.varDecl->u.declScalar->type->type == A_structTypeKind)
                    {
                        globalVarMap.emplace(*v->u.varDeclStmt->u.varDecl->u.declScalar->id,
                            Name_newname_struct(Temp_newlabel_named(*v->u.varDeclStmt->u.varDecl->u.declScalar->id),*v->u.varDeclStmt->u.varDecl->u.declScalar->type->u.structType));
                        TempDef def(TempType::STRUCT_TEMP,0,*v->u.varDeclStmt->u.varDecl->u.declScalar->type->u.structType);
                        defs.push_back(L_Globaldef(*v->u.varDeclStmt->u.varDecl->u.declScalar->id,def,vector<int>()));
                    }
                    else
                    {
                        globalVarMap.emplace(*v->u.varDeclStmt->u.varDecl->u.declScalar->id,
                            Name_newname_int(Temp_newlabel_named(*v->u.varDeclStmt->u.varDecl->u.declScalar->id)));
                        TempDef def(TempType::INT_TEMP,0);
                        defs.push_back(L_Globaldef(*v->u.varDeclStmt->u.varDecl->u.declScalar->id,def,vector<int>()));
                    }
                }
                else if(v->u.varDeclStmt->u.varDecl->kind == A_varDeclArrayKind)
                {
                    if(v->u.varDeclStmt->u.varDecl->u.declArray->type->type == A_structTypeKind)
                    {
                        globalVarMap.emplace(*v->u.varDeclStmt->u.varDecl->u.declArray->id,
                            Name_newname_struct_ptr(Temp_newlabel_named(*v->u.varDeclStmt->u.varDecl->u.declArray->id),v->u.varDeclStmt->u.varDecl->u.declArray->len,*v->u.varDeclStmt->u.varDecl->u.declArray->type->u.structType));
                        TempDef def(TempType::STRUCT_PTR,v->u.varDeclStmt->u.varDecl->u.declArray->len,*v->u.varDeclStmt->u.varDecl->u.declArray->type->u.structType);
                        defs.push_back(L_Globaldef(*v->u.varDeclStmt->u.varDecl->u.declArray->id,def,vector<int>()));
                    }
                    else
                    {
                        globalVarMap.emplace(*v->u.varDeclStmt->u.varDecl->u.declArray->id,
                            Name_newname_int_ptr(Temp_newlabel_named(*v->u.varDeclStmt->u.varDecl->u.declArray->id),v->u.varDeclStmt->u.varDecl->u.declArray->len));
                        TempDef def(TempType::INT_PTR,v->u.varDeclStmt->u.varDecl->u.declArray->len);
                        defs.push_back(L_Globaldef(*v->u.varDeclStmt->u.varDecl->u.declArray->id,def,vector<int>()));
                    }
                }
                else
                {
                    assert(0);
                }
            }
            else if(v->u.varDeclStmt->kind == A_varDefKind)
            {
                if(v->u.varDeclStmt->u.varDef->kind == A_varDefScalarKind)
                {
                    if(v->u.varDeclStmt->u.varDef->u.defScalar->type->type == A_structTypeKind)
                    {
                        globalVarMap.emplace(*v->u.varDeclStmt->u.varDef->u.defScalar->id,
                            Name_newname_struct(Temp_newlabel_named(*v->u.varDeclStmt->u.varDef->u.defScalar->id),*v->u.varDeclStmt->u.varDef->u.defScalar->type->u.structType));
                        TempDef def(TempType::STRUCT_TEMP,0,*v->u.varDeclStmt->u.varDef->u.defScalar->type->u.structType);
                        defs.push_back(L_Globaldef(*v->u.varDeclStmt->u.varDef->u.defScalar->id,def,vector<int>()));
                    }
                    else
                    {
                        globalVarMap.emplace(*v->u.varDeclStmt->u.varDef->u.defScalar->id,
                            Name_newname_int(Temp_newlabel_named(*v->u.varDeclStmt->u.varDef->u.defScalar->id)));
                        TempDef def(TempType::INT_TEMP,0);
                        vector<int> init;
                        init.push_back(ast2llvmRightVal_first(v->u.varDeclStmt->u.varDef->u.defScalar->val));
                        defs.push_back(L_Globaldef(*v->u.varDeclStmt->u.varDef->u.defScalar->id,def,init));
                    }
                }
                else if(v->u.varDeclStmt->u.varDef->kind == A_varDefArrayKind)
                {
                    if(v->u.varDeclStmt->u.varDef->u.defArray->type->type == A_structTypeKind)
                    {
                        globalVarMap.emplace(*v->u.varDeclStmt->u.varDef->u.defArray->id,
                            Name_newname_struct_ptr(Temp_newlabel_named(*v->u.varDeclStmt->u.varDef->u.defArray->id),v->u.varDeclStmt->u.varDef->u.defArray->len,*v->u.varDeclStmt->u.varDef->u.defArray->type->u.structType));
                        TempDef def(TempType::STRUCT_PTR,v->u.varDeclStmt->u.varDef->u.defArray->len,*v->u.varDeclStmt->u.varDef->u.defArray->type->u.structType);
                        defs.push_back(L_Globaldef(*v->u.varDeclStmt->u.varDef->u.defArray->id,def,vector<int>()));
                    }
                    else
                    {
                        globalVarMap.emplace(*v->u.varDeclStmt->u.varDef->u.defArray->id,
                            Name_newname_int_ptr(Temp_newlabel_named(*v->u.varDeclStmt->u.varDef->u.defArray->id),v->u.varDeclStmt->u.varDef->u.defArray->len));
                        TempDef def(TempType::INT_PTR,v->u.varDeclStmt->u.varDef->u.defArray->len);
                        vector<int> init;
                        for(auto &el : v->u.varDeclStmt->u.varDef->u.defArray->vals)
                        {
                            init.push_back(ast2llvmRightVal_first(el));
                        }
                        defs.push_back(L_Globaldef(*v->u.varDeclStmt->u.varDef->u.defArray->id,def,init));
                    }
                }
                else
                {
                    assert(0);
                }
            }
            else
            {
                assert(0);
            }
            break;
        }
        case A_programStructDefKind:
        {
            StructInfo si;
            int off = 0;
            vector<TempDef> members;
            for(const auto &decl : v->u.structDef->varDecls)
            {
                if(decl->kind == A_varDeclScalarKind)
                {
                    if(decl->u.declScalar->type->type == A_structTypeKind)
                    {
                        TempDef def(TempType::STRUCT_TEMP,0,*decl->u.declScalar->type->u.structType);
                        MemberInfo info(off++,def);
                        si.memberinfos.emplace(*decl->u.declScalar->id,info);
                        members.push_back(def);
                    }
                    else
                    {
                        TempDef def(TempType::INT_TEMP,0);
                        MemberInfo info(off++,def);
                        si.memberinfos.emplace(*decl->u.declScalar->id,info);
                        members.push_back(def);
                    }
                }
                else if(decl->kind == A_varDeclArrayKind)
                {
                    if(decl->u.declArray->type->type == A_structTypeKind)
                    {
                        TempDef def(TempType::STRUCT_PTR,decl->u.declArray->len,*decl->u.declArray->type->u.structType);
                        MemberInfo info(off++,def);
                        si.memberinfos.emplace(*decl->u.declArray->id,info);
                        members.push_back(def);
                    }
                    else
                    {
                        TempDef def(TempType::INT_PTR,decl->u.declArray->len);
                        MemberInfo info(off++,def);
                        si.memberinfos.emplace(*decl->u.declArray->id,info);
                        members.push_back(def);
                    }
                }
                else
                {
                    assert(0);
                }
            }
            structInfoMap.emplace(*v->u.structDef->id,std::move(si));
            defs.push_back(L_Structdef(*v->u.structDef->id,members));
            break;
        }
        case A_programFnDeclStmtKind:
        {
            FuncType type;
            if(v->u.fnDeclStmt->fnDecl->type == nullptr)
            {
                type.type = ReturnType::VOID_TYPE;
            }
            else if(v->u.fnDeclStmt->fnDecl->type->type == A_nativeTypeKind)
            {
                type.type = ReturnType::INT_TYPE;
            }
            else if(v->u.fnDeclStmt->fnDecl->type->type == A_structTypeKind)
            {
                type.type = ReturnType::STRUCT_TYPE;
                type.structname = *v->u.fnDeclStmt->fnDecl->type->u.structType;
            }
            else
            {
                assert(0);
            }
            if(funcReturnMap.find(*v->u.fnDeclStmt->fnDecl->id) == funcReturnMap.end())
                funcReturnMap.emplace(*v->u.fnDeclStmt->fnDecl->id,std::move(type));
            vector<TempDef> args;
            for(const auto & decl : v->u.fnDeclStmt->fnDecl->paramDecl->varDecls)
            {
                if(decl->kind == A_varDeclScalarKind)
                {
                    if(decl->u.declScalar->type->type == A_structTypeKind)
                    {
                        TempDef def(TempType::STRUCT_PTR,0,*decl->u.declScalar->type->u.structType);
                        args.push_back(def);
                    }
                    else
                    {
                        TempDef def(TempType::INT_TEMP,0);
                        args.push_back(def);
                    }
                }
                else if(decl->kind == A_varDeclArrayKind)
                {
                    if(decl->u.declArray->type->type == A_structTypeKind)
                    {
                        TempDef def(TempType::STRUCT_PTR,-1,*decl->u.declArray->type->u.structType);
                        args.push_back(def);
                    }
                    else
                    {
                        TempDef def(TempType::INT_PTR,-1);
                        args.push_back(def);
                    }
                }
                else
                {
                    assert(0);
                }
            }
            defs.push_back(L_Funcdecl(*v->u.fnDeclStmt->fnDecl->id,args,type));
            break;
        }
        case A_programFnDefKind:
        {
            if(funcReturnMap.find(*v->u.fnDef->fnDecl->id) == funcReturnMap.end())
            {
                FuncType type;
                if(v->u.fnDef->fnDecl->type == nullptr)
                {
                    type.type = ReturnType::VOID_TYPE;
                }
                else if(v->u.fnDef->fnDecl->type->type == A_nativeTypeKind)
                {
                    type.type = ReturnType::INT_TYPE;
                }
                else if(v->u.fnDef->fnDecl->type->type == A_structTypeKind)
                {
                    type.type = ReturnType::STRUCT_TYPE;
                    type.structname = *v->u.fnDef->fnDecl->type->u.structType;
                }
                else
                {
                    assert(0);
                }
                funcReturnMap.emplace(*v->u.fnDef->fnDecl->id,std::move(type));
            }
            break;
        }
        default:
            assert(0);
            break;
        }
    }
    return defs;
}

std::vector<Func_local*> ast2llvmProg_second(aA_program p)
{
    vector<Func_local*> funcs;
    for(const auto & v : p->programElements)
    {
        switch (v->kind)
        {
        case A_programNullStmtKind:
        {
            break;
        }
        case A_programVarDeclStmtKind:
        {
            break;
        }
        case A_programStructDefKind:
        {
            break;
        }
        case A_programFnDeclStmtKind:
        {
            break;
        }
        case A_programFnDefKind:
        {
            funcs.push_back(ast2llvmFunc(v->u.fnDef));
            break;
        }
        default:
            assert(0);
            break;
        }
    }
    return funcs;
}

Func_local* ast2llvmFunc(aA_fnDef f)
{
    emit_irs.clear();
    emit_irs.push_back(L_Label(Temp_newlabel_named(*f->fnDecl->id)));
    localVarMap.clear();
    vector<Temp_temp*> args;
    for(const auto & decl : f->fnDecl->paramDecl->varDecls)
    {
        bool scale_int = false;
        if(decl->kind == A_varDeclScalarKind)
        {
            if(decl->u.declScalar->type->type == A_structTypeKind)
            {
                Temp_temp* arg = Temp_newtemp_struct_ptr(0,*decl->u.declScalar->type->u.structType);
                args.push_back(arg);
            }
            else
            {
                Temp_temp* arg = Temp_newtemp_int();
                args.push_back(arg);
                Temp_temp* temp_ptr =  Temp_newtemp_int_ptr(0);
                AS_operand* arg_ptr = AS_Operand_Temp(temp_ptr);
                emit_irs.push_back(L_Alloca(arg_ptr));
                emit_irs.push_back(L_Store(AS_Operand_Temp(arg),arg_ptr));
                localVarMap.emplace(*decl->u.declScalar->id,temp_ptr);
                scale_int = true;
            }
        }
        else if(decl->kind == A_varDeclArrayKind)
        {
            if(decl->u.declArray->type->type == A_structTypeKind)
            {
                args.push_back(Temp_newtemp_struct_ptr(-1,*decl->u.declArray->type->u.structType));
            }
            else
            {
                args.push_back(Temp_newtemp_int_ptr(-1));
            }
        }
        else
        {
            assert(0);
        }
        if(!scale_int)
        {
            localVarMap.emplace(*decl->u.declScalar->id,args.back());
        }
    }
    
    for(const auto &stmt : f->stmts)
    {
        ast2llvmBlock(stmt,nullptr,nullptr);
    }

    return new Func_local(*f->fnDecl->id,funcReturnMap[*f->fnDecl->id],args,emit_irs);
}

void ast2llvmBlock(aA_codeBlockStmt b,Temp_label *con_label,Temp_label *bre_label)
{
    if(b->kind == A_codeBlockStmtType::A_assignStmtKind){
        AS_operand* left = ast2llvmLeftVal(b->u.assignStmt->leftVal);
        AS_operand* right = ast2llvmRightVal(b->u.assignStmt->rightVal);
        emit_irs.push_back(L_Store(right,left));
    }
    else if(b->kind == A_codeBlockStmtType::A_breakStmtKind){
        emit_irs.push_back(L_Jump(bre_label));
    }
    else if(b->kind == A_codeBlockStmtType::A_continueStmtKind){
        emit_irs.push_back(L_Jump(con_label));
    }
    else if(b->kind == A_codeBlockStmtType::A_returnStmtKind){
        AS_operand* ret;
        if(b->u.returnStmt->retVal != nullptr)
            ret = ast2llvmRightVal(b->u.returnStmt->retVal);
        else 
            ret = nullptr;
        emit_irs.push_back(L_Ret(ret));
    }
    else if(b->kind == A_codeBlockStmtType::A_ifStmtKind){
        Temp_label* true_label = Temp_newlabel();
        Temp_label* false_label = Temp_newlabel();
        Temp_label* next_label = Temp_newlabel();
        AS_operand* condition = ast2llvmBoolExpr(b->u.whileStmt->boolExpr, true_label, false_label);
        emit_irs.push_back(L_Cjump(condition,true_label,false_label));
        emit_irs.push_back(L_Label(true_label));
        for(const auto &stmt : b->u.ifStmt->ifStmts)
        {
            ast2llvmBlock(stmt,con_label,bre_label);
        }
        emit_irs.push_back(L_Jump(next_label));
        emit_irs.push_back(L_Label(false_label));
        for(const auto &stmt : b->u.ifStmt->elseStmts)
        {
            ast2llvmBlock(stmt,con_label,bre_label);
        }
        emit_irs.push_back(L_Label(next_label));
    }
    else if(b->kind == A_codeBlockStmtType::A_whileStmtKind){
        Temp_label* test_label = Temp_newlabel();
        Temp_label *true_label = Temp_newlabel();
        Temp_label *false_label = Temp_newlabel(); 
        emit_irs.push_back(L_Label(test_label));
        AS_operand* condition = ast2llvmBoolExpr(b->u.whileStmt->boolExpr, true_label, false_label);
        emit_irs.push_back(L_Cjump(condition,true_label,false_label));
        emit_irs.push_back(L_Label(true_label));
        for(const auto &stmt : b->u.whileStmt->whileStmts)
        {
            ast2llvmBlock(stmt,test_label,false_label);
        }
        emit_irs.push_back(L_Jump(test_label));
        emit_irs.push_back(L_Label(false_label));
    }
    else if(b->kind == A_codeBlockStmtType::A_callStmtKind){
        vector<AS_operand*> args;
        for(const auto &arg : b->u.callStmt->fnCall->vals)
        {
            AS_operand* arg_init = ast2llvmRightVal(arg);
            args.push_back(arg_init);
        }
        if(funcReturnMap[*b->u.callStmt->fnCall->fn].type == ReturnType::VOID_TYPE)
        {
            emit_irs.push_back(L_Voidcall(*b->u.callStmt->fnCall->fn,args));
        }
        else if(funcReturnMap[*b->u.callStmt->fnCall->fn].type == ReturnType::INT_TYPE)
        {
            AS_operand* ret = AS_Operand_Temp(Temp_newtemp_int());
            emit_irs.push_back(L_Call(*b->u.callStmt->fnCall->fn,ret,args));
        }
        else if(funcReturnMap[*b->u.callStmt->fnCall->fn].type == ReturnType::STRUCT_TYPE)
        {
            AS_operand* ret = AS_Operand_Temp(Temp_newtemp_struct(funcReturnMap[*b->u.callStmt->fnCall->fn].structname));
            emit_irs.push_back(L_Call(*b->u.callStmt->fnCall->fn,ret,args));
        }
    }
    else if (b->kind == A_codeBlockStmtType::A_varDeclStmtKind)
    {
        if(b->u.varDeclStmt->kind == A_varDeclKind)
        {
            if(b->u.varDeclStmt->u.varDecl->kind == A_varDeclScalarKind)
            {
                string name = *b->u.varDeclStmt->u.varDecl->u.declScalar->id;
                if(b->u.varDeclStmt->u.varDecl->u.declScalar->type->type == A_dataType::A_structTypeKind)
                {    
                    Temp_temp* temp = Temp_newtemp_struct_ptr(0,*b->u.varDeclStmt->u.varDecl->u.declScalar->type->u.structType);
                    localVarMap[name] = temp;
                    emit_irs.push_back(L_Alloca(AS_Operand_Temp(temp)));
                }
                else if(b->u.varDeclStmt->u.varDecl->u.declScalar->type->type == A_dataType::A_nativeTypeKind)
                {
                    Temp_temp* temp = Temp_newtemp_int_ptr(0);
                    localVarMap[name] = temp;
                    emit_irs.push_back(L_Alloca(AS_Operand_Temp(temp)));
                }
                else
                {
                    assert(0);
                }
            }
            else if(b->u.varDeclStmt->u.varDecl->kind == A_varDeclArrayKind)
            {
                string name = *b->u.varDeclStmt->u.varDecl->u.declArray->id;
                if(b->u.varDeclStmt->u.varDecl->u.declArray->type->type == A_dataType::A_structTypeKind)
                {
                    Temp_temp* temp = Temp_newtemp_struct_ptr(b->u.varDeclStmt->u.varDecl->u.declArray->len,*b->u.varDeclStmt->u.varDecl->u.declArray->type->u.structType);
                    localVarMap[name] = temp;
                    emit_irs.push_back(L_Alloca(AS_Operand_Temp(temp)));
                }
                else if(b->u.varDeclStmt->u.varDecl->u.declArray->type->type == A_dataType::A_nativeTypeKind)
                {
                    Temp_temp* temp = Temp_newtemp_int_ptr(b->u.varDeclStmt->u.varDecl->u.declArray->len);
                    localVarMap[name] = temp;
                    emit_irs.push_back(L_Alloca(AS_Operand_Temp(temp)));
                }
                else
                {
                    assert(0);
                }
            }
            else
            {
                assert(0);
            }
        }
        else if(b->u.varDeclStmt->kind == A_varDefKind)
        {
            if(b->u.varDeclStmt->u.varDef->kind == A_varDefScalarKind)
            {
                string name = *b->u.varDeclStmt->u.varDef->u.defScalar->id;
                if(b->u.varDeclStmt->u.varDef->u.defScalar->type->type == A_structTypeKind)
                {
                    Temp_temp* temp = Temp_newtemp_struct_ptr(0,*b->u.varDeclStmt->u.varDef->u.defScalar->type->u.structType);
                    localVarMap[name] = temp;
                    AS_operand* left = AS_Operand_Temp(temp);
                    emit_irs.push_back(L_Alloca(left));
                    AS_operand* right = ast2llvmRightVal(b->u.varDeclStmt->u.varDef->u.defScalar->val);
                    emit_irs.push_back(L_Store(right,left));
                }
                else if(b->u.varDeclStmt->u.varDef->u.defScalar->type->type == A_nativeTypeKind)
                {
                    Temp_temp* temp = Temp_newtemp_int_ptr(0);
                    localVarMap[name] = temp;
                    AS_operand* left = AS_Operand_Temp(temp);
                    emit_irs.push_back(L_Alloca(left));
                    AS_operand* right = ast2llvmRightVal(b->u.varDeclStmt->u.varDef->u.defScalar->val);
                    emit_irs.push_back(L_Store(right,left));
                }
                else
                {
                    assert(0);
                }
                
            }
            else if(b->u.varDeclStmt->u.varDef->kind == A_varDefArrayKind)
            {
                string name = *b->u.varDeclStmt->u.varDef->u.defArray->id;
                if(b->u.varDeclStmt->u.varDef->u.defArray->type->type == A_structTypeKind)
                {
                    Temp_temp* temp = Temp_newtemp_struct_ptr(b->u.varDeclStmt->u.varDef->u.defArray->len,*b->u.varDeclStmt->u.varDef->u.defArray->type->u.structType);
                    localVarMap[name] = temp;
                    AS_operand* left = AS_Operand_Temp(temp);
                    emit_irs.push_back(L_Alloca(left));
                    int i = 0;
                    for(auto &el : b->u.varDeclStmt->u.varDef->u.defArray->vals)
                    {
                        AS_operand* arr_element = AS_Operand_Temp(Temp_newtemp_struct(*b->u.varDeclStmt->u.varDef->u.defArray->type->u.structType));
                        AS_operand* index = AS_Operand_Const(i++);
                        emit_irs.push_back(L_Gep(arr_element,left,index));
                        AS_operand* right = ast2llvmRightVal(el);
                        emit_irs.push_back(L_Store(right,arr_element));
                    }
                }
                else if(b->u.varDeclStmt->u.varDef->u.defArray->type->type == A_nativeTypeKind)
                {
                    Temp_temp* temp = Temp_newtemp_int_ptr(b->u.varDeclStmt->u.varDef->u.defArray->len);
                    localVarMap[name] = temp;
                    AS_operand* left = AS_Operand_Temp(temp);
                    emit_irs.push_back(L_Alloca(left));
                    int i = 0;
                    for(auto &el : b->u.varDeclStmt->u.varDef->u.defArray->vals)
                    {
                        AS_operand* arr_element = AS_Operand_Temp(Temp_newtemp_int());
                        AS_operand* index = AS_Operand_Const(i++);
                        emit_irs.push_back(L_Gep(arr_element,left,index));
                        AS_operand* right = ast2llvmRightVal(el);
                        emit_irs.push_back(L_Store(right,arr_element));
                    }
                }
                else
                {
                    assert(0);
                }
            }
            else
            {
                assert(0);
            }
        }
    }
    else if (b->kind == A_codeBlockStmtType::A_nullStmtKind)
    {
        
    }
    else 
    {
       assert(0);
    }
}

AS_operand* ast2llvmRightVal(aA_rightVal r)
{
    if(r->kind == A_rightValType::A_arithExprValKind)
    {
        return ast2llvmArithExpr(r->u.arithExpr);
    }
    else if(r->kind == A_rightValType::A_boolExprValKind)
    {
        Temp_label* true_label = Temp_newlabel();
        Temp_label* false_label = Temp_newlabel();
        AS_operand* right = ast2llvmBoolExpr(r->u.boolExpr,true_label,false_label);
        AS_operand* ret = AS_Operand_Temp(Temp_newtemp_int_ptr(0));
        emit_irs.push_back(L_Alloca(ret));
        emit_irs.push_back(L_Cjump(right,true_label,false_label));
        Temp_label* next_label = Temp_newlabel();
        emit_irs.push_back(L_Label(true_label));
        emit_irs.push_back(L_Store(AS_Operand_Const(1),ret));
        emit_irs.push_back(L_Jump(next_label));
        emit_irs.push_back(L_Label(false_label));
        emit_irs.push_back(L_Store(AS_Operand_Const(0),ret));
        emit_irs.push_back(L_Jump(next_label));
        AS_operand* ret_val = AS_Operand_Temp(Temp_newtemp_int());
        emit_irs.push_back(L_Label(next_label));
        emit_irs.push_back(L_Load(ret_val,ret));
        return ret_val;
    }
    else
    {
        assert(0);
    }
}

AS_operand* ast2llvmLeftVal(aA_leftVal l)
{
    if(l->kind == A_leftValType::A_varValKind)
    {
        if(localVarMap.find(*l->u.id) != localVarMap.end())
        {
            return AS_Operand_Temp(localVarMap[*l->u.id]);
        }
        else if(globalVarMap.find(*l->u.id) != globalVarMap.end())
        {
            return AS_Operand_Name(globalVarMap[*l->u.id]);
        }
        else
        {
            assert(0);
        }
    }
    else if(l->kind == A_leftValType::A_arrValKind)
    {
        AS_operand* base_ptr = ast2llvmLeftVal(l->u.arrExpr->arr);  
        AS_operand* index = ast2llvmIndexExpr(l->u.arrExpr->idx);
        AS_operand* new_ptr; 
        if(base_ptr->kind == OperandKind::TEMP)
        {
            if(base_ptr->u.TEMP->type == TempType::INT_PTR)
                new_ptr = AS_Operand_Temp(Temp_newtemp_int_ptr(0));
            else if(base_ptr->u.TEMP->type == TempType::STRUCT_PTR)
                new_ptr = AS_Operand_Temp(Temp_newtemp_struct_ptr(0,base_ptr->u.TEMP->structname));
            else
                assert(0);
        }
        else if(base_ptr->kind == OperandKind::NAME)
        {
            if(base_ptr->u.NAME->type == TempType::INT_PTR)
                new_ptr = AS_Operand_Temp(Temp_newtemp_int_ptr(0));
            else if(base_ptr->u.NAME->type == TempType::STRUCT_PTR)
                new_ptr = AS_Operand_Temp(Temp_newtemp_struct_ptr(0,base_ptr->u.NAME->structname));
            else
                assert(0);
        }
        else
        {
            assert(0);
        }
        emit_irs.push_back(L_Gep(new_ptr,base_ptr,index));
        return new_ptr;
    }
    else if(l->kind == A_leftValType::A_memberValKind)
    {
        AS_operand* base_struct_ptr = ast2llvmLeftVal(l->u.memberExpr->structId);
        unordered_map<string, StructInfo>::iterator it;
        string name;
        if(base_struct_ptr->kind == OperandKind::TEMP)
            name = base_struct_ptr->u.TEMP->structname;
        else if(base_struct_ptr->kind == OperandKind::NAME)
            name = base_struct_ptr->u.NAME->structname;
        else
            assert(0);
        if((it = structInfoMap.find(name)) != structInfoMap.end())
        {
            unordered_map<string, MemberInfo>::iterator it_mem;
            if((it_mem = it->second.memberinfos.find(*l->u.memberExpr->memberId)) != it->second.memberinfos.end())
            {
                AS_operand* index = AS_Operand_Const(it_mem->second.offset);
                AS_operand* new_mem_ptr;
                if(it_mem->second.def.kind == TempType::INT_TEMP)
                    new_mem_ptr = AS_Operand_Temp(Temp_newtemp_int());
                else if(it_mem->second.def.kind == TempType::INT_PTR)
                    new_mem_ptr = AS_Operand_Temp(Temp_newtemp_int_ptr(it_mem->second.def.len));
                else if(it_mem->second.def.kind == TempType::STRUCT_TEMP){
                    new_mem_ptr = AS_Operand_Temp(Temp_newtemp_struct_ptr(0,it_mem->second.def.structname));
                }
                else if(it_mem->second.def.kind == TempType::STRUCT_PTR)
                    new_mem_ptr = AS_Operand_Temp(Temp_newtemp_struct_ptr(it_mem->second.def.len,it_mem->second.def.structname));
                else
                    assert(0);
                emit_irs.push_back(L_Gep(new_mem_ptr,base_struct_ptr,index));
                return new_mem_ptr;
            }
            else 
                assert(0);
        }
        else
            assert(0);
    }
    else
    {
        assert(0);
    }
}

AS_operand* ast2llvmIndexExpr(aA_indexExpr index)
{
    if(index->kind == A_indexExprKind::A_numIndexKind)
    {
        return AS_Operand_Const(index->u.num);
    }
    else if(index->kind == A_indexExprKind::A_idIndexKind)
    {
        unordered_map<string, Temp_temp*>::iterator it_temp;
        unordered_map<string, Name_name*>::iterator it_name;
        if((it_temp = localVarMap.find(*index->u.id)) != localVarMap.end())
        {
            if(it_temp->second->type == TempType::INT_PTR){
                AS_operand* ret = AS_Operand_Temp(Temp_newtemp_int());
                emit_irs.push_back(L_Load(ret,AS_Operand_Temp(it_temp->second)));
                return ret;
            }
            else if(it_temp->second->type == TempType::STRUCT_PTR){
                AS_operand* ret = AS_Operand_Temp(Temp_newtemp_struct(it_temp->second->structname));
                emit_irs.push_back(L_Load(ret,AS_Operand_Temp(it_temp->second)));
                return ret;
            }
            else
                assert(0);
        }
        else if(( it_name = globalVarMap.find(*index->u.id)) != globalVarMap.end())
        {
            AS_operand* ope_name = AS_Operand_Name(globalVarMap[*index->u.id]);
            AS_operand* ope_dst;
            if(globalVarMap[*index->u.id]->type == TempType::INT_TEMP){
                ope_dst = AS_Operand_Temp(Temp_newtemp_int());
                emit_irs.push_back(L_Load(ope_dst,ope_name));
            }
            else
                ope_dst = ope_name;
            return ope_dst;
        }
        else
        {
            assert(0);
        }
    }
    else
    {
        assert(0);
    }
}

AS_operand* ast2llvmBoolExpr(aA_boolExpr b,Temp_label *true_label,Temp_label *false_label)
{
    AS_operand* right;
    if(b->kind == A_boolExprType::A_boolBiOpExprKind)
    {
        return ast2llvmBoolBiOpExpr(b->u.boolBiOpExpr,true_label,false_label);
    }
    else if(b->kind == A_boolExprType::A_boolUnitKind)
    {
        return ast2llvmBoolUnit(b->u.boolUnit,true_label,false_label);
    }
    else
    {
        assert(0);
    }
}

AS_operand* ast2llvmBoolBiOpExpr(aA_boolBiOpExpr b,Temp_label *true_label,Temp_label *false_label)
{     
    AS_operand* left = ast2llvmBoolExpr(b->left,true_label,false_label);
    if(b->op == A_boolBiOp::A_and)
    {
        Temp_label* and_true = Temp_newlabel();
        emit_irs.push_back(L_Cjump(left,and_true,false_label));
        emit_irs.push_back(L_Label(and_true));
    }
    else if(b->op == A_boolBiOp::A_or)
    {
        Temp_label* or_false = Temp_newlabel();
        emit_irs.push_back(L_Cjump(left,true_label,or_false));
        emit_irs.push_back(L_Label(or_false));
    }
    else
    {
        assert(0);
    }
    AS_operand* right = ast2llvmBoolExpr(b->right,true_label,false_label);
    return right;
}

AS_operand* ast2llvmBoolUnit(aA_boolUnit b,Temp_label *true_label,Temp_label *false_label)
{
    if(b->kind == A_boolUnitType::A_boolExprKind)
    {
       return ast2llvmBoolExpr(b->u.boolExpr,true_label,false_label);
    }
    else if(b->kind == A_boolUnitType::A_boolUOpExprKind)
    {
        if(b->u.boolUOpExpr->op == A_boolUOp::A_not)
        {
            return ast2llvmBoolUnit(b->u.boolUOpExpr->cond,false_label,true_label);
        }
        else
        {
            assert(0);
        }
    }
    else if(b->kind == A_boolUnitType::A_comOpExprKind)
    {
        return ast2llvmComOpExpr(b->u.comExpr,true_label,false_label);
    }
    else
    {
        assert(0);
    }
}

AS_operand* ast2llvmComOpExpr(aA_comExpr c,Temp_label *true_label,Temp_label *false_label)
{
    AS_operand* left = ast2llvmExprUnit(c->left);
    AS_operand* right = ast2llvmExprUnit(c->right);    
    AS_operand* ret = AS_Operand_Temp(Temp_newtemp_int());
    if(c->op == A_comOp::A_eq)
    {
        emit_irs.push_back(L_Cmp(L_relopKind::T_eq,left,right,ret));
    }
    else if(c->op == A_comOp::A_ne)
    {
        emit_irs.push_back(L_Cmp(L_relopKind::T_ne,left,right,ret));
    }
    else if(c->op == A_comOp::A_lt)
    {
        emit_irs.push_back(L_Cmp(L_relopKind::T_lt,left,right,ret));
    }
    else if(c->op == A_comOp::A_le)
    {
        emit_irs.push_back(L_Cmp(L_relopKind::T_le,left,right,ret));
    }
    else if(c->op == A_comOp::A_gt)
    {
        emit_irs.push_back(L_Cmp(L_relopKind::T_gt,left,right,ret));
    }
    else if(c->op == A_comOp::A_ge)
    {
        emit_irs.push_back(L_Cmp(L_relopKind::T_ge,left,right,ret));
    }
    else
    {
        assert(0);
    }
    return ret;
}

AS_operand* ast2llvmArithBiOpExpr(aA_arithBiOpExpr a)
{
    AS_operand* ret;
    if(a->op == A_arithBiOp::A_add)
    {
        AS_operand* left = ast2llvmArithExpr(a->left);
        AS_operand* right = ast2llvmArithExpr(a->right);
        ret = AS_Operand_Temp(Temp_newtemp_int());
        emit_irs.push_back(L_Binop(L_binopKind::T_plus,left,right,ret));
    }
    else if(a->op == A_arithBiOp::A_sub)
    {
        AS_operand* left = ast2llvmArithExpr(a->left);
        AS_operand* right = ast2llvmArithExpr(a->right);
        ret = AS_Operand_Temp(Temp_newtemp_int());
        emit_irs.push_back(L_Binop(L_binopKind::T_minus,left,right,ret));
    }
    else if(a->op == A_arithBiOp::A_mul)
    {
        AS_operand* left = ast2llvmArithExpr(a->left); 
        AS_operand* right = ast2llvmArithExpr(a->right);
        ret = AS_Operand_Temp(Temp_newtemp_int());
        emit_irs.push_back(L_Binop(L_binopKind::T_mul,left,right,ret));
    }
    else if(a->op == A_arithBiOp::A_div)
    {
        AS_operand* left = ast2llvmArithExpr(a->left);
        AS_operand* right = ast2llvmArithExpr(a->right);
        ret = AS_Operand_Temp(Temp_newtemp_int());
        emit_irs.push_back(L_Binop(L_binopKind::T_div,left,right,ret));
    }
    return ret;
}

AS_operand* ast2llvmArithUExpr(aA_arithUExpr a)
{
    AS_operand* expr = ast2llvmExprUnit(a->expr);
    if(a->op == A_arithUOp::A_neg)
    {
        AS_operand* ret = AS_Operand_Temp(Temp_newtemp_int());
        emit_irs.push_back(L_Binop(L_binopKind::T_minus,AS_Operand_Const(0),expr,ret));
        return ret;
    }
    else
    {
        assert(0);
    }
}

AS_operand* ast2llvmArithExpr(aA_arithExpr a)
{
    if(a->kind == A_arithExprType::A_arithBiOpExprKind)
    {
        return ast2llvmArithBiOpExpr(a->u.arithBiOpExpr);
    }
    else if(a->kind == A_arithExprType::A_exprUnitKind)
    {
        return ast2llvmExprUnit(a->u.exprUnit);
    }
    else
    {
        assert(0);
    }
}

AS_operand* ast2llvmExprUnit(aA_exprUnit e)
{
    if(e->kind == A_exprUnitType::A_numExprKind)
    {
        return AS_Operand_Const(e->u.num);
    }
    else if(e->kind == A_exprUnitType::A_idExprKind)
    {
        if(localVarMap.find(*e->u.id) != localVarMap.end())
        {
            Temp_temp* temp = localVarMap[*e->u.id];
            AS_operand* ope_temp = AS_Operand_Temp(temp);
            if(temp->type == TempType::INT_PTR && temp->len == 0)
            {
                AS_operand* ope_new = AS_Operand_Temp(Temp_newtemp_int());
                emit_irs.push_back(L_Load(ope_new,ope_temp));
                return ope_new;
            }
            else
                return ope_temp;
        }
        else if(globalVarMap.find(*e->u.id) != globalVarMap.end())
        {
            AS_operand* ope_name = AS_Operand_Name(globalVarMap[*e->u.id]);
            AS_operand* ope_dst;
            if(globalVarMap[*e->u.id]->type == TempType::INT_TEMP){
                ope_dst = AS_Operand_Temp(Temp_newtemp_int());
                emit_irs.push_back(L_Load(ope_dst,ope_name));
            }
            else
                ope_dst = ope_name;
            return ope_dst;
        }
        else
        {
            assert(0);
        }
    }
    else if(e->kind == A_exprUnitType::A_arrayExprKind)
    {
        AS_operand* index = ast2llvmIndexExpr(e->u.arrayExpr->idx);
        AS_operand* base_ptr = ast2llvmLeftVal(e->u.arrayExpr->arr);
        AS_operand* new_ptr;
        if(base_ptr->kind == OperandKind::TEMP)
        {
            if(base_ptr->u.TEMP->type == TempType::INT_PTR)
                new_ptr = AS_Operand_Temp(Temp_newtemp_int_ptr(0));
            else if(base_ptr->u.TEMP->type == TempType::STRUCT_PTR)
                new_ptr = AS_Operand_Temp(Temp_newtemp_struct_ptr(0,base_ptr->u.TEMP->structname));
            else
                assert(0);
        }
        else if(base_ptr->kind == OperandKind::NAME)
        {
            if(base_ptr->u.NAME->type == TempType::INT_PTR)
                new_ptr = AS_Operand_Temp(Temp_newtemp_int_ptr(0));
            else if(base_ptr->u.NAME->type == TempType::STRUCT_PTR)
                new_ptr = AS_Operand_Temp(Temp_newtemp_struct_ptr(0,base_ptr->u.NAME->structname));
            else
                assert(0);
        }
        else
        {
            assert(0);
        }
        emit_irs.push_back(L_Gep(new_ptr,base_ptr,index));
        AS_operand* new_ptr_val = AS_Operand_Temp(Temp_newtemp_int());
        emit_irs.push_back(L_Load(new_ptr_val,new_ptr));
        return new_ptr_val;
    }
    else if(e->kind == A_exprUnitType::A_memberExprKind)
    {
        AS_operand* base_struct_ptr = ast2llvmLeftVal(e->u.memberExpr->structId);
        unordered_map<string, StructInfo>::iterator it;
        string name;
        if(base_struct_ptr->kind == OperandKind::TEMP)
        {
            name = base_struct_ptr->u.TEMP->structname;
            if((it = structInfoMap.find(name)) != structInfoMap.end())
            {
                unordered_map<string, MemberInfo>::iterator it_mem;
                if((it_mem = it->second.memberinfos.find(*e->u.memberExpr->memberId)) != it->second.memberinfos.end())
                {
                    AS_operand* index = AS_Operand_Const(it_mem->second.offset);
                    AS_operand* new_mem_ptr = AS_Operand_Temp(Temp_newtemp_struct_ptr(0,base_struct_ptr->u.TEMP->structname));
                    emit_irs.push_back(L_Gep(new_mem_ptr,base_struct_ptr,index));
                    AS_operand* new_mem_ptr_val;
                    if(it_mem->second.def.kind == TempType::INT_TEMP)
                        new_mem_ptr_val = AS_Operand_Temp(Temp_newtemp_int());
                    else if(it_mem->second.def.kind == TempType::STRUCT_TEMP)
                        new_mem_ptr_val = AS_Operand_Temp(Temp_newtemp_struct(it_mem->second.def.structname));
                    else if(it_mem->second.def.kind == TempType::INT_PTR)
                        new_mem_ptr_val = AS_Operand_Temp(Temp_newtemp_int_ptr(it_mem->second.def.len));
                    else if(it_mem->second.def.kind == TempType::STRUCT_PTR)
                        new_mem_ptr_val = AS_Operand_Temp(Temp_newtemp_struct_ptr(it_mem->second.def.len,it_mem->second.def.structname));
                    else
                       assert(0);
                    emit_irs.push_back(L_Load(new_mem_ptr_val,new_mem_ptr));
                    return new_mem_ptr_val;
                }
                else
                    assert(0);
            }
            else
                assert(0);
        }
        else if(base_struct_ptr->kind == OperandKind::NAME)
        {
            name = base_struct_ptr->u.NAME->structname;
            if((it = structInfoMap.find(name)) != structInfoMap.end())
            {
                unordered_map<string, MemberInfo>::iterator it_mem;
                if((it_mem = it->second.memberinfos.find(*e->u.memberExpr->memberId)) != it->second.memberinfos.end())
                {
                    AS_operand* index = AS_Operand_Const(it_mem->second.offset);
                    AS_operand* new_mem_ptr = AS_Operand_Temp(Temp_newtemp_struct_ptr(0,base_struct_ptr->u.NAME->structname));
                    emit_irs.push_back(L_Gep(new_mem_ptr,base_struct_ptr,index));
                    AS_operand* new_mem_ptr_val;
                    if(it_mem->second.def.kind == TempType::INT_TEMP)
                        new_mem_ptr_val = AS_Operand_Temp(Temp_newtemp_int());
                    else if(it_mem->second.def.kind == TempType::STRUCT_TEMP)
                        new_mem_ptr_val = AS_Operand_Temp(Temp_newtemp_struct(it_mem->second.def.structname));
                    else if(it_mem->second.def.kind == TempType::INT_PTR)
                        new_mem_ptr_val = AS_Operand_Temp(Temp_newtemp_int_ptr(it_mem->second.def.len));
                    else if(it_mem->second.def.kind == TempType::STRUCT_PTR)
                        new_mem_ptr_val = AS_Operand_Temp(Temp_newtemp_struct_ptr(it_mem->second.def.len,it_mem->second.def.structname));
                    else
                       assert(0);
                    emit_irs.push_back(L_Load(new_mem_ptr_val,new_mem_ptr));
                    return new_mem_ptr_val;
                }
                else
                    assert(0);
            }
            else
                assert(0);
        }
        else
            assert(0);
    }
    else if(e->kind == A_exprUnitType::A_arithUExprKind)
    {
        return ast2llvmArithUExpr(e->u.arithUExpr);
    }
    else if (e->kind == A_exprUnitType::A_fnCallKind)
    {
        vector<AS_operand*> args;  
        for(int i = 0;i < e->u.callExpr->vals.size();i++)
        {
            AS_operand* arg_init = ast2llvmRightVal(e->u.callExpr->vals[i]);
            args.push_back(arg_init);
        }
        AS_operand* ret;
        if(funcReturnMap[*e->u.callExpr->fn].type == ReturnType::INT_TYPE)
        {
            ret = AS_Operand_Temp(Temp_newtemp_int());
        }
        else if(funcReturnMap[*e->u.callExpr->fn].type == ReturnType::STRUCT_TYPE)
        {
            ret = AS_Operand_Temp(Temp_newtemp_struct(funcReturnMap[*e->u.callExpr->fn].structname));
        }
        else
        {
            assert(0);
        }
        emit_irs.push_back(L_Call(*e->u.callExpr->fn,ret,args));
        return ret;
    }
    else if(e->kind == A_exprUnitType::A_arithExprKind){
        return ast2llvmArithExpr(e->u.arithExpr);
    }
    else
    {
        assert(0);
    }    
}

LLVMIR::L_func* ast2llvmFuncBlock(Func_local *f)
{
    list<L_block*> blocks;
    list<L_stm*> irs_of_block;
    for(auto &ir : f->irs)
    {
        if(ir->type == L_StmKind::T_LABEL)
        {
            if(irs_of_block.size() != 0)
            {
                irs_of_block.push_back(L_Jump(ir->u.LABEL->label));
                blocks.push_back(L_Block(irs_of_block));
                irs_of_block.clear();
            }
            irs_of_block.push_back(ir);
        }
        else if (ir->type == L_StmKind::T_CJUMP || ir->type == L_StmKind::T_JUMP || ir->type == L_StmKind::T_RETURN)
        {
            irs_of_block.push_back(ir);
            if(irs_of_block.front()->type != L_StmKind::T_LABEL)
            {
                irs_of_block.push_front(L_Label(Temp_newlabel()));
            }
            blocks.push_back(L_Block(irs_of_block));
            irs_of_block.clear();
        }
        else
        {
            irs_of_block.push_back(ir);
        }
        if(ir == f->irs.back())
        {
            if(irs_of_block.size() != 0)
            {
                if(irs_of_block.front()->type != L_StmKind::T_LABEL)
                {
                    irs_of_block.push_front(L_Label(Temp_newlabel()));
                }
                if(f->ret.type == ReturnType::VOID_TYPE)
                    irs_of_block.push_back(L_Ret(nullptr));
                else
                    irs_of_block.push_back(L_Ret(AS_Operand_Const(0)));
                blocks.push_back(L_Block(irs_of_block));
                irs_of_block.clear();
            }
        }
    }
    return new L_func(f->name,f->ret,f->args,blocks);;
}

void ast2llvm_moveAlloca(LLVMIR::L_func *f)
{
    auto first_block = f->blocks.front();
    for(auto i = ++f->blocks.begin();i != f->blocks.end();++i)
    {
        for(auto it = (*i)->instrs.begin();it != (*i)->instrs.end();)
        {
            if((*it)->type == L_StmKind::T_ALLOCA)
            {
                first_block->instrs.insert(++first_block->instrs.begin(),*it);
                it = (*i)->instrs.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
}