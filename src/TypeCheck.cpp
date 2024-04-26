#include "TypeCheck.h"

//global tabels
//typeMap func2retType; // function name to return type

// global token ids to type
typeMap g_token2Type; 
vector<string> func_declared;
vector<string> func_defined;

// local token ids to type, since func param can override global param
typeMap funcparam_token2Type;
vector<typeMap*> local_token2Type;
string current_func;

paramMemberMap func2Param;
paramMemberMap struct2Members;

static tc_type Bool_aAType = []() {
    tc_type bool_type = new tc_type_;
    aA_type bool_type_aA = new aA_type_;
    bool_type_aA->type = A_dataType::A_structTypeKind;
    bool_type_aA->u.structType = new string("bool");
    bool_type->type = bool_type_aA;
    bool_type->isVarArrFunc = 0;
    return bool_type;
}();

string get_type_name(tc_type type) {
  if (type == nullptr) return "nullptr";
  switch (type->isVarArrFunc) {
    case 0: 
        if(type->type->type == A_dataType::A_nativeTypeKind &&
            type->type->u.nativeType == A_nativeType::A_intTypeKind){
            return "int";
        }else if(type->type->type == A_dataType::A_structTypeKind){
            return *(type->type->u.structType);
        }
    case 1:
        return "array";
    case 2:
        return "function";
  }
  return "unknown";
}

// private util functions
void error_print(std::ostream& out, A_pos p, string info)
{
    out << "Typecheck error in line " << p->line << ", col " << p->col << ": " << info << std::endl;
    exit(0);
}

void print_token_map(typeMap* map){
    for(auto it = map->begin(); it != map->end(); it++){
        std::cout << it->first << " : ";
        switch (it->second->type->type)
        {
        case A_dataType::A_nativeTypeKind:
            switch (it->second->type->u.nativeType)
            {
            case A_nativeType::A_intTypeKind:
                std::cout << "int";
                break;
            default:
                break;
            }
            break;
        case A_dataType::A_structTypeKind:
            std::cout << *(it->second->type->u.structType);
            break;
        default:
            break;
        }
        switch(it->second->isVarArrFunc){
            case 0:
                std::cout << " scalar";
                break;
            case 1:
                std::cout << " array";
                break;
            case 2:
                std::cout << " function";
                break;
        }
        std::cout << std::endl;
    }
}

string errmsg_fndecl(string name, A_pos pos){
    // print g_token2Type
    // print_token_map(&g_token2Type);
    return "function definition on line " + 
            std::to_string(pos->line) + 
            " doesn't match the declaration on line " + 
            std::to_string(g_token2Type[name]->type->pos->line) + 
            ".";
}

string errmsg_assign_not_def(string name, A_pos pos){
    return "var " +
            name +
            " on line " +
            std::to_string(pos->line) +
            " col " +
            std::to_string(pos->col) +
            " is not defined.";
}

string errmsg_assign_struct_not_def(string name, A_pos pos){
    return "struct " +
            name +
            " on line " +
            std::to_string(pos->line) +
            " col " +
            std::to_string(pos->col) +
            " is not defined.";
}

string errmsg_assign_member_not_def(string name, A_pos pos){
    return "member '" +
            name +
            "' on line " +
            std::to_string(pos->line) +
            " col " +
            std::to_string(pos->col) +
            " is not defined.";
}

string errmsg_fnCall_param_check(string name, A_pos pos){
    return "Parameters passed to Function call " + 
            name + 
            " on line " + 
            std::to_string(pos->line) +
            " col " +
            std::to_string(pos->col) +
            " do not match the definition!";
}

string errmsg_comOp(tc_type left, tc_type right, A_pos pos){
    return get_type_name(left) +
            " is not comparable with "
            + get_type_name(right) +
            ".";
}

void print_paramMemberMap(paramMemberMap* map){
    std::cout<<"paramMemberMap:"<<std::endl;
    for(auto it = map->begin(); it != map->end(); it++){
        std::cout << it->first << std::endl;
    }
}




void print_token_maps(){
    std::cout << "global token2Type:" << std::endl;
    print_token_map(&g_token2Type);
    std::cout << "local token2Type:" << std::endl;
    print_token_map(&funcparam_token2Type);
}


bool comp_aA_type(aA_type target, aA_type t){
    if(!target || !t)
        return false;
    if(target->type != t->type)
        return false;
    if(target->type == A_dataType::A_nativeTypeKind)
        if(target->u.nativeType != t->u.nativeType)
            return false;
    if(target->type == A_dataType::A_structTypeKind)
        if(*target->u.structType != *t->u.structType)
            return false;
    return true;
}


bool comp_tc_type(tc_type target, tc_type t){
    // if(target->isVarArrFunc==0){
    //     std::cout << "isVarArrFunc is nullptr target"  << std::endl;
    // }
    // if(t->isVarArrFunc==0)
    //     std::cout << "isVarArrFunc is nullptr t" << std::endl;
    
    if(!target || !t)
        return false;
    
    // arr kind first
    if (target->isVarArrFunc && t->isVarArrFunc == 0)
        return false;
    
    // if target type is nullptr, alwayse ok
    return comp_aA_type(target->type, t->type);
}


tc_type tc_Type(aA_type t, uint isVarArrFunc){
    tc_type ret = new tc_type_;
    ret->type = t;
    ret->isVarArrFunc = isVarArrFunc;
    return ret;
}
tc_type tc_Type(aA_type t, uint isVarArrFunc, int len){
    tc_type ret = new tc_type_;
    ret->type = t;
    ret->isVarArrFunc = isVarArrFunc;
    ret->array_len = len;
    return ret;
}
// tc_type tc_Type_test(aA_type t, uint isVarArrFunc){
//     tc_type ret = new tc_type_;
//     ret->type = t;
//     ret->isVarArrFunc = isVarArrFunc;
//     // std::cout << ret->isVarArrFunc << std::endl;
//     return ret;
// }


tc_type tc_Type(aA_varDecl vd){
    if(vd->kind == A_varDeclType::A_varDeclScalarKind)
        return tc_Type(vd->u.declScalar->type, 0);
    else if(vd->kind == A_varDeclType::A_varDeclArrayKind)
        return tc_Type(vd->u.declArray->type, 1, vd->u.declArray->len);
    return nullptr;
}
// tc_type tc_Type_test(aA_varDecl vd){
//     if(vd->kind == A_varDeclType::A_varDeclScalarKind){
//         // std::cout<<"scalar"<<std::endl;
//         // if(!tc_Type_test(vd->u.declScalar->type, 0)->isVarArrFunc){
//         //     std::cout<<"no"<<std::endl;
//         // }
//         tc_type temp = tc_Type_test(vd->u.declScalar->type, 0);
//         std::cout << temp->isVarArrFunc << std::endl;
//         return tc_Type(vd->u.declScalar->type, 0);
//     }
//     else if(vd->kind == A_varDeclType::A_varDeclArrayKind)
//         return tc_Type(vd->u.declArray->type, 1);
//     return nullptr;
// }


// public functions
void check_Prog(std::ostream& out, aA_program p)
{
    std::cout << "########################" << std::endl;
    for (auto ele : p->programElements)
    {
        if(ele->kind == A_programVarDeclStmtKind){
            check_VarDecl(out, ele->u.varDeclStmt);
        }else if (ele->kind == A_programStructDefKind){
            check_StructDef(out, ele->u.structDef);
        }
    }
    
    for (auto ele : p->programElements){
        if(ele->kind == A_programFnDeclStmtKind){
            string name = *ele->u.fnDeclStmt->fnDecl->id;
            for (auto it = func_declared.begin(); it != func_declared.end(); ++it) {
                if(*it == name){
                    error_print(out, ele->pos, "Function " + name + " on line " + 
                        std::to_string(ele->pos->line) + " col " + 
                        std::to_string(ele->pos->col) + " has been declared.");
                }
            }
            check_FnDeclStmt(out, ele->u.fnDeclStmt);
            func_declared.push_back(name);
        }
        else if (ele->kind == A_programFnDefKind){
            check_FnDecl(out, ele->u.fnDef->fnDecl);
            string name = *ele->u.fnDef->fnDecl->id;
            bool is_declared = false;
            for (auto it = func_declared.begin(); it != func_declared.end(); ++it) {
                if(*it == name){
                    is_declared = true;
                    break;
                }
            }
            if(!is_declared){
                func_declared.push_back(*ele->u.fnDef->fnDecl->id);
            }
        }
    }

    for (auto ele : p->programElements){
        if(ele->kind == A_programFnDefKind){
            string name = *ele->u.fnDef->fnDecl->id;
            for (auto it = func_defined.begin(); it != func_defined.end(); ++it) {
                if(*it == name){
                    error_print(out, ele->pos, "Function " + name + " on line " + 
                        std::to_string(ele->pos->line) + " col " + 
                        std::to_string(ele->pos->col) + " has been defined.");
                }
            }
            check_FnDef(out, ele->u.fnDef);
            func_defined.push_back(name);
        }
        else if (ele->kind == A_programNullStmtKind){
            // do nothing
        }
    }

    out << "Typecheck passed!" << std::endl;
    return;
}

bool check_declared(std::ostream& out, string name, A_pos pos){
    if(funcparam_token2Type.find(name) != funcparam_token2Type.end()){
        error_print(out, pos, "Local variable '" + name + "' on line " + 
            std::to_string(pos->line) + " col " + 
            std::to_string(pos->col) + "  duplicates with function params.");
        return true;
    }
    for (auto it = local_token2Type.rbegin(); it != local_token2Type.rend(); ++it) {
        typeMap* map = *it;
        auto found = map->find(name);
        if (found != map->end()) {
            error_print(out, pos, "Local variable '" + name + "' on line " + 
                std::to_string(pos->line) + " col " + 
                std::to_string(pos->col) + "  has been declared.");
            return true;
        }
    }
    auto found = g_token2Type.find(name);
    if (found != g_token2Type.end()) {
        error_print(out, pos, "Local variable '" + name + "' on line " + 
            std::to_string(pos->line) + " col " + 
            std::to_string(pos->col) + "  duplicates with global variables.");
        return true;
    }
    return false;
}

tc_type get_rval_type(std::ostream& out, aA_rightVal rv){
    if(rv->kind == A_rightValType::A_arithExprValKind){
        return check_ArithExpr(out, rv->u.arithExpr);
    }else if(rv->kind == A_rightValType::A_boolExprValKind){
        check_BoolExpr(out, rv->u.boolExpr);
        return Bool_aAType;
    }
    std::cout << "right value is not arith or bool!" << std::endl;
    return nullptr;
}

void check_VarDecl(std::ostream& out, aA_varDeclStmt vd)
{
    if (!vd)
        return;
    string name;
    // print_token_maps();
    // std::cout<<name<<std::endl;
    if (vd->kind == A_varDeclStmtType::A_varDeclKind){
        // decl only
        aA_varDecl vdecl = vd->u.varDecl;
        if(vdecl->kind == A_varDeclType::A_varDeclScalarKind){
            name = *vdecl->u.declScalar->id;
            /* fill code here*/
            if(local_token2Type.empty()){
                g_token2Type[name] = tc_Type(vdecl);
            }else{
                if(!check_declared(out, name, vdecl->pos)){
                    (*local_token2Type.back())[name] = tc_Type(vdecl);
                }
            }
        }else if (vdecl->kind == A_varDeclType::A_varDeclArrayKind){
            name = *vdecl->u.declArray->id;
            /* fill code here*/
            if(local_token2Type.empty()){
                g_token2Type[name] = tc_Type(vdecl);
            }else{
                (*local_token2Type.back())[name] = tc_Type(vdecl);
            }
        }
    }
    else if (vd->kind == A_varDeclStmtType::A_varDefKind){
        // decl and def
        aA_varDef vdef = vd->u.varDef;
        if (vdef->kind == A_varDefType::A_varDefScalarKind){
            name = *vdef->u.defScalar->id;
            /* fill code here, allow omited type */
            tc_type left_type;
            tc_type right_type;
            if(vdef->u.defScalar->type){
                left_type = tc_Type(vdef->u.defScalar->type, 0);
                right_type = get_rval_type(out, vdef->u.defScalar->val);
                if(!comp_tc_type(left_type, right_type)){
                    error_print(out, vdef->pos, "Type mismatch in assignment! Can not assign to "
                        + get_type_name(left_type) + " " + name + " when defining it.");
                }
            }else{
                right_type = get_rval_type(out, vdef->u.defScalar->val);
            }
            if(local_token2Type.empty()){
                g_token2Type[name] = right_type;
            }else{
                if(!check_declared(out, name, vdef->pos)){
                    (*local_token2Type.back())[name] = right_type;
                }
            }
        }else if (vdef->kind == A_varDefType::A_varDefArrayKind){
            name = *vdef->u.defArray->id;
            /* fill code here, allow omited type */
            tc_type left_type;
            tc_type right_type;
            if(vdef->u.defArray->type){
                left_type = tc_Type(vdef->u.defArray->type, 1);
                right_type = get_rval_type(out, vdef->u.defArray->vals[0]);
                right_type->isVarArrFunc = 1;
                right_type->array_len = vdef->u.defArray->vals.size();
                if(!comp_tc_type(left_type, right_type)){
                    error_print(out, vdef->pos, "Type mismatch in assignment! Can not assign to "
                        + get_type_name(left_type) + " " + name + " when defining it.");
                }
                if(vdef->u.defArray->len != vdef->u.defArray->vals.size()){
                    error_print(out, vdef->pos, "Array length mismatch!");
                }
            }else{
                right_type = get_rval_type(out, vdef->u.defArray->vals[0]);
            }
            if(local_token2Type.empty()){
                g_token2Type[name] = right_type;
            }else{
                (*local_token2Type.back())[name] = right_type;
            }
        }
    }
    return;
}


void check_StructDef(std::ostream& out, aA_structDef sd)
{
    if (!sd)
        return;
    string name = *sd->id;
    if (struct2Members.find(name) != struct2Members.end())
        error_print(out, sd->pos, "This id is already defined!");
    struct2Members[name] = &(sd->varDecls);
    return;
}


void check_FnDecl(std::ostream& out, aA_fnDecl fd)
{
    if (!fd)
        return;
    string name = *fd->id;
    // print_paramMemberMap(&func2Param);
    // std::cout << name << std::endl;
    // if already declared, should match
    // print_paramMemberMap(&func2Param);
    if(func2Param.find(name) != func2Param.end()){
        // is function ret val matches
        /* fill code here */
        if (g_token2Type[name]->type && !comp_aA_type(g_token2Type[name]->type, fd->type)) {
            string error_message = errmsg_fndecl(name, fd->pos);
            error_print(out, fd->pos, error_message);
        }
        // is function params matches decl
        /* fill code here */
        vector<aA_varDecl>* prevParams = func2Param[name];
        vector<aA_varDecl>* curParams = &fd->paramDecl->varDecls;
        if (prevParams->size() != curParams->size()) {
            string error_message = errmsg_fndecl(name, fd->pos);
            error_print(out, fd->pos, error_message);
        } else {
            for (size_t i = 0; i < prevParams->size(); ++i) {
                aA_varDecl prevVarDecl = (*prevParams)[i];
                aA_varDecl curVarDecl = (*curParams)[i];
                aA_type prevType, curType;
                if(prevVarDecl->kind == A_varDeclType::A_varDeclScalarKind){
                    prevType = prevVarDecl->u.declScalar->type;
                }else if(prevVarDecl->kind == A_varDeclType::A_varDeclArrayKind){
                    prevType = prevVarDecl->u.declArray->type;
                }
                if(curVarDecl->kind == A_varDeclType::A_varDeclScalarKind){
                    curType = curVarDecl->u.declScalar->type;
                }else if(curVarDecl->kind == A_varDeclType::A_varDeclArrayKind){
                    curType = curVarDecl->u.declArray->type;
                }
                if(!comp_aA_type(prevType, curType)){
                    string error_message = errmsg_fndecl(name, fd->pos);
                    error_print(out, fd->pos, error_message);
                    break;
                }
            }
        }
    }else{
        // if not declared
        /* fill code here */
        func2Param[name] = &fd->paramDecl->varDecls;
        g_token2Type[name] = tc_Type(fd->type, 2);
    }
    return;
}


void check_FnDeclStmt(std::ostream& out, aA_fnDeclStmt fd)
{
    if (!fd)
        return;
    check_FnDecl(out, fd->fnDecl);
    return;
}

void check_FnDef(std::ostream& out, aA_fnDef fd)
{
    if (!fd)
        return;
    // should match if declared
    check_FnDecl(out, fd->fnDecl);
    // add params to local tokenmap, func params override global ones
    for (aA_varDecl vd : fd->fnDecl->paramDecl->varDecls)
    {
        /* fill code here */
        if(vd->kind == A_varDeclType::A_varDeclScalarKind){
            funcparam_token2Type[*vd->u.declScalar->id] = tc_Type(vd);
        }else if(vd->kind == A_varDeclType::A_varDeclArrayKind){
            funcparam_token2Type[*vd->u.declArray->id] = tc_Type(vd);
        }
    }

    /* fill code here */
    local_token2Type.push_back(new typeMap());

    current_func = *fd->fnDecl->id;

    for (aA_codeBlockStmt stmt : fd->stmts)
    {
        check_CodeblockStmt(out, stmt);
        // return value type should match
        /* fill code here */
        if(stmt->kind == A_codeBlockStmtType::A_returnStmtKind){
            tc_type should = g_token2Type[*fd->fnDecl->id];
            tc_type is = get_rval_type(out, stmt->u.returnStmt->retVal);
            if(!comp_aA_type(should->type, is->type)){
                error_print(out, stmt->pos, "Return type mismatch!");
            }
        }
    }

    funcparam_token2Type.clear();
    local_token2Type.pop_back();
    current_func.clear();
    return;
}


void check_CodeblockStmt(std::ostream& out, aA_codeBlockStmt cs){
    if(!cs)
        return;
    // variables declared in a code block should not duplicate with outer ones.
    switch (cs->kind)
    {
    case A_codeBlockStmtType::A_varDeclStmtKind:
        check_VarDecl(out, cs->u.varDeclStmt);
        break;
    case A_codeBlockStmtType::A_assignStmtKind:
        check_AssignStmt(out, cs->u.assignStmt);
        break;
    case A_codeBlockStmtType::A_ifStmtKind:
        check_IfStmt(out, cs->u.ifStmt);
        break;
    case A_codeBlockStmtType::A_whileStmtKind:
        check_WhileStmt(out, cs->u.whileStmt);
        break;
    case A_codeBlockStmtType::A_callStmtKind:
        check_CallStmt(out, cs->u.callStmt);
        break;
    case A_codeBlockStmtType::A_returnStmtKind:
        check_ReturnStmt(out, cs->u.returnStmt);
        break;
    default:
        break;
    }
    return;
}

tc_type check_var_exist(std::ostream& out, string name){
    // Check function params
    if(funcparam_token2Type.find(name) != funcparam_token2Type.end()){
        return funcparam_token2Type[name];
    }

    // Check local scopes
    for (auto it = local_token2Type.rbegin(); it != local_token2Type.rend(); ++it) {
        typeMap* map = *it;
        auto found = map->find(name);
        if (found != map->end()) {
            return found->second;
        }
    }
    // Variable not found in local scopes, check global scope
    auto found = g_token2Type.find(name);
    if (found != g_token2Type.end()) {
        return found->second;
    }

    return nullptr;
}


tc_type check_lval_scalar(std::ostream& out, string name, A_pos pos, bool* isExist){
    tc_type type = check_var_exist(out, name);
    if(!type){
        string error_message = errmsg_assign_not_def(name, pos);
        error_print(out, pos, error_message);
        return type;
    }
    *isExist = true;
    return type;
}

tc_type check_lval_array(std::ostream& out, string name, aA_assignStmt as, bool* isExist){
    tc_type type = check_var_exist(out, name);
    if(!type){
        string error_message = errmsg_assign_not_def(name, as->leftVal->pos);
        error_print(out, as->leftVal->pos, error_message);
        return type;
    }
    // Check the array index
    aA_indexExpr idx_expr = as->leftVal->u.arrExpr->idx;
    if(idx_expr->kind == A_indexExprKind::A_idIndexKind){
        check_var_exist(out, *idx_expr->u.id);
    }
    *isExist = true;
    return type;
}

tc_type check_struct_exist(std::ostream& out, aA_memberExpr me){
    string struct_name = *me->structId->u.id;
    A_pos struct_pos = me->structId->pos;
    tc_type struct_type = check_var_exist(out, struct_name);
    if(!struct_type){
        string error_message = errmsg_assign_not_def(struct_name, struct_pos);
        error_print(out, struct_pos, error_message);
        return nullptr;
    }
    if(struct_type->type->type != A_dataType::A_structTypeKind){
        error_print(out, struct_pos, struct_name + " is not a struct.");
        return nullptr;
    }
    string struct_type_name = *struct_type->type->u.structType;
    if(struct2Members.find(struct_type_name) == struct2Members.end()){
        string error_message = errmsg_assign_struct_not_def(struct_type_name, struct_pos);
        error_print(out, struct_pos, error_message);
        return nullptr;
    }
    return struct_type;
}

tc_type check_member_exist(std::ostream& out, aA_memberExpr me, tc_type struct_type, bool* isExist){
    string struct_name = *me->structId->u.id;
    string struct_type_name = *struct_type->type->u.structType;
    string member_name = *me->memberId;
    A_pos member_pos = me->pos;
    // Check if the member is defined
    vector<aA_varDecl>* members = struct2Members[struct_type_name];
    for(aA_varDecl member : *members){
        if(member->kind == A_varDeclType::A_varDeclScalarKind){
            if(*member->u.declScalar->id == member_name){
                *isExist = true;
                return tc_Type(member);
            }
        }else if(member->kind == A_varDeclType::A_varDeclArrayKind){
            if(*member->u.declArray->id == member_name){
                *isExist = true;
                return tc_Type(member);
            }
        }
    }
    string error_message = errmsg_assign_member_not_def(member_name, member_pos);
    error_print(out, member_pos, error_message);
    return nullptr;
}

void check_AssignStmt(std::ostream& out, aA_assignStmt as){
    if(!as)
        return;
    string name;
    tc_type deduced_type; // deduced type if type is omitted at decl
    bool is_need_deduce = false;
    tc_type left_val_type;
    bool isExist = false;
    string type;
    switch (as->leftVal->kind)
    {
        case A_leftValType::A_varValKind:{
            name = *as->leftVal->u.id;
            // std::cout<<name<<std::endl;
            /* fill code here */
            left_val_type = check_lval_scalar(out, name, as->leftVal->pos, &isExist);
            if(!isExist){
                return;
            }else if(left_val_type->type){
                deduced_type = left_val_type;
            }else{        
                // deduced_type = nullptr;
                is_need_deduce = true;
            }
        }
            break;
        case A_leftValType::A_arrValKind:{
            name = *as->leftVal->u.arrExpr->arr->u.id;
            /* fill code here */
            left_val_type = check_lval_array(out, name, as, &isExist);
            if(!isExist){
                return;
            }
            else if(left_val_type->type){
                deduced_type = left_val_type;
            }else{
                // deduced_type = nullptr;
                is_need_deduce = true;
            }
            // deduced_type->type = left_val_type->type;
        }
            break;
        case A_leftValType::A_memberValKind:{
            /* fill code here */
            tc_type struct_type = check_struct_exist(out, as->leftVal->u.memberExpr);
            if(struct_type){
                left_val_type = check_member_exist(out, as->leftVal->u.memberExpr, struct_type, &isExist);
            }else{
                left_val_type = nullptr;
            }
            if(!isExist){
                return;
            }
            else if(left_val_type->type){
                deduced_type = left_val_type;
            }else{
                // deduced_type = nullptr;
                is_need_deduce = true;
            }
            // deduced_type->type = left_val_type->type;
        }
            break;
    }
    tc_type right_type = get_rval_type(out, as->rightVal);
    if(!right_type){
        return;
    }
    if(!is_need_deduce){
        // no need to deduce
        if(!comp_tc_type(deduced_type, right_type)){
            error_print(out, as->pos, "Type mismatch in assignment! Can not assign to "
                + get_type_name(left_val_type) + " " + name + ".");    
        }
    }else{
        // deduce type
        deduced_type = right_type;
        g_token2Type[name] = deduced_type;
        if(local_token2Type.empty()){
            g_token2Type[name] = deduced_type;
        }else{
            (*local_token2Type.back())[name] = deduced_type;
        }
    }

    return;
}


void check_ArrayExpr(std::ostream& out, aA_arrayExpr ae){
    if(!ae)
        return;
    string name = *ae->arr->u.id;
    // check array name
    /* fill code here */
    tc_type type = check_var_exist(out, name);
    if(!type){
        string error_message = errmsg_assign_not_def(name, ae->pos);
        error_print(out, ae->pos, error_message);
    }
    // check index
    /* fill code here */
    aA_indexExpr idx_expr = ae->idx;
    if(idx_expr->kind == A_indexExprKind::A_idIndexKind){
        check_var_exist(out, *idx_expr->u.id);
    }
    return;
}

 
tc_type check_MemberExpr(std::ostream& out, aA_memberExpr me){
    // check if the member exists and return the tyep of the member
    if(!me)
        return nullptr;
    // check struct name
    /* fill code here */
    string struct_name = *me->structId->u.id;
    A_pos struct_pos = me->structId->pos;
    tc_type struct_type = check_var_exist(out, struct_name);
    if(!struct_type){
        string error_message = errmsg_assign_not_def(struct_name, struct_pos);
        error_print(out, struct_pos, error_message);
        return nullptr;
    }
    string struct_type_name = *struct_type->type->u.structType;
    if(struct2Members.find(struct_type_name) == struct2Members.end()){
        string error_message = errmsg_assign_struct_not_def(struct_type_name, struct_pos);
        error_print(out, struct_pos, error_message);
        return nullptr;
    }
    // check member name
    /* fill code here */
    string member_name = *me->memberId;
    A_pos member_pos = me->pos;
    vector<aA_varDecl>* members = struct2Members[struct_type_name];
    for(aA_varDecl member : *members){
        if(member->kind == A_varDeclType::A_varDeclScalarKind){
            if(*member->u.declScalar->id == member_name){
                return tc_Type(member);
            }
        }else if(member->kind == A_varDeclType::A_varDeclArrayKind){
            if(*member->u.declArray->id == member_name){
                return tc_Type(member);
            }
        }
    }
    string error_message = errmsg_assign_member_not_def(member_name, member_pos);
    error_print(out, member_pos, error_message);
    return nullptr;
}


void check_IfStmt(std::ostream& out, aA_ifStmt is){
    if(!is)
        return;
    check_BoolExpr(out, is->boolExpr);
    /* fill code here, take care of variable scope */
    local_token2Type.push_back(new typeMap());

    for(aA_codeBlockStmt s : is->ifStmts){
        check_CodeblockStmt(out, s);
    }
    
    /* fill code here */    
    local_token2Type.pop_back();
    local_token2Type.push_back(new typeMap());
    for(aA_codeBlockStmt s : is->elseStmts){
        check_CodeblockStmt(out, s);
    }
    /* fill code here */
    local_token2Type.pop_back();
    return;
}


void check_BoolExpr(std::ostream& out, aA_boolExpr be){
    if(!be)
        return;
    switch (be->kind)
    {
    case A_boolExprType::A_boolBiOpExprKind:
        check_BoolExpr(out, be->u.boolBiOpExpr->left);
        check_BoolExpr(out, be->u.boolBiOpExpr->right);
        break;
    case A_boolExprType::A_boolUnitKind:
        check_BoolUnit(out, be->u.boolUnit);
        break;
    default:
        break;
    }
    return;
}


void check_BoolUnit(std::ostream& out, aA_boolUnit bu){
    if(!bu)
        return;
    switch (bu->kind)
    {
        case A_boolUnitType::A_comOpExprKind:{
            /* fill code here */
            tc_type left_expr_type = check_ExprUnit(out, bu->u.comExpr->left);
            tc_type right_expr_type = check_ExprUnit(out, bu->u.comExpr->right);
            if(!left_expr_type || !right_expr_type){
                error_print(out, bu->pos, "Type mismatch in comparison operation!");
            }else if(left_expr_type->type->type == A_dataType::A_structTypeKind ||
                    right_expr_type->type->type == A_dataType::A_structTypeKind){
                string error_message = errmsg_comOp(left_expr_type, right_expr_type, bu->pos);
                error_print(out, bu->pos, error_message);
            }else if(!comp_tc_type(left_expr_type, right_expr_type)){
                string error_message = errmsg_comOp(left_expr_type, right_expr_type, bu->pos);
                error_print(out, bu->pos, error_message);
            }
        }
            break;
        case A_boolUnitType::A_boolExprKind:
            check_BoolExpr(out, bu->u.boolExpr);
            break;
        case A_boolUnitType::A_boolUOpExprKind:
            check_BoolUnit(out, bu->u.boolUOpExpr->cond);
            break;
        default:
            break;
    }
    return;
}

tc_type get_array_type(string name){
    for(auto it = local_token2Type.rbegin(); it != local_token2Type.rend(); ++it){
        typeMap* map = *it;
        auto found = map->find(name);
        if(found != map->end()){
            return found->second;
        }
    }
    auto found = g_token2Type.find(name);
    if(found != g_token2Type.end()){
        return found->second;
    }
    return nullptr;
}

tc_type check_ExprUnit(std::ostream& out, aA_exprUnit eu){
    // return the aA_type of expr eu
    if(!eu)
        return nullptr;
    tc_type ret;
    switch (eu->kind)
    {
        case A_exprUnitType::A_idExprKind:{
            /* fill code here */
            ret = check_var_exist(out, *eu->u.id);
        }
            break;
        case A_exprUnitType::A_numExprKind:{
            aA_type numt = new aA_type_;
            numt->pos = eu->pos;
            numt->type = A_dataType::A_nativeTypeKind;
            numt->u.nativeType = A_nativeType::A_intTypeKind;
            ret = tc_Type(numt, 0);
        }
            break;
        case A_exprUnitType::A_fnCallKind:{
            check_FuncCall(out, eu->u.callExpr);
            // check_FuncCall will check if the function is defined
            /* fill code here */
            ret = g_token2Type[*eu->u.callExpr->fn];
        }
            break;
        case A_exprUnitType::A_arrayExprKind:{
            check_ArrayExpr(out, eu->u.arrayExpr);
            /* fill code here */
            ret = get_array_type(*eu->u.arrayExpr->arr->u.id);
        }
            break;
        case A_exprUnitType::A_memberExprKind:{
            ret = check_MemberExpr(out, eu->u.memberExpr);
        }
            break;
        case A_exprUnitType::A_arithExprKind:{
            ret = check_ArithExpr(out, eu->u.arithExpr);
        }
            break;
        case A_exprUnitType::A_arithUExprKind:{
            ret = check_ExprUnit(out, eu->u.arithUExpr->expr);
        }
            break;
    }
    return ret;
}


tc_type check_ArithExpr(std::ostream& out, aA_arithExpr ae){
    if(!ae)
        return nullptr;
    tc_type ret;
    switch (ae->kind)
    {
        case A_arithExprType::A_arithBiOpExprKind:{
            ret = check_ArithExpr(out, ae->u.arithBiOpExpr->left);
            tc_type rightTyep = check_ArithExpr(out, ae->u.arithBiOpExpr->right);
            if(!ret || !rightTyep){
                error_print(out, ae->pos, "variable in arithExpr not defined!");
            }

            if(ret->type->type > 0 || ret->type->type != A_dataType::A_nativeTypeKind || ret->type->u.nativeType != A_nativeType::A_intTypeKind ||
            rightTyep->type->type > 0 || rightTyep->type->type != A_dataType::A_nativeTypeKind || rightTyep->type->u.nativeType != A_nativeType::A_intTypeKind){
                error_print(out, ae->pos, "Only int can be arithmetic expression operation values!");
            }
        }
            break;
        case A_arithExprType::A_exprUnitKind:
            ret = check_ExprUnit(out, ae->u.exprUnit);
            break;
    }
    return ret;
}

int get_array_len(string name){
    for(auto it = local_token2Type.rbegin(); it != local_token2Type.rend(); ++it){
        typeMap* map = *it;
        auto found = map->find(name);
        if(found != map->end()){
            return found->second->array_len;
        }
    }
    auto found = g_token2Type.find(name);
    if(found != g_token2Type.end()){
        return found->second->array_len;
    }
    return -1;
}


void check_FuncCall(std::ostream& out, aA_fnCall fc){
    if(!fc)
        return;
    // check if function defined
    string func_name = *fc->fn;
    /* fill code here */
    if(func2Param.find(func_name) == func2Param.end()){
        error_print(out, fc->pos, "Function " + func_name + " on line " + 
                    std::to_string(fc->pos->line) + " col " + 
                    std::to_string(fc->pos->col) + " is not defined!");
        return;
    }
        
    // check if parameter list matches
    vector<aA_varDecl>* params = func2Param[func_name];
    if(params->size() != fc->vals.size()){
        string error_message = errmsg_fnCall_param_check(func_name, fc->pos);
        error_print(out, fc->pos, error_message);
    }
    for(int i = 0; i < fc->vals.size(); i++){
        /* fill code here */
        aA_rightVal rv = fc->vals[i];
        aA_varDecl param = (*params)[i];
        tc_type param_type = tc_Type(param);
        if(param->kind == A_varDeclType::A_varDeclScalarKind){
            aA_varDeclScalar scalar = param->u.declScalar;
            tc_type rv_type = get_rval_type(out, rv);
            if(!comp_tc_type(param_type, rv_type)){
                string error_message = errmsg_fnCall_param_check(func_name, fc->pos);
                error_print(out, fc->pos, error_message);
            }
        }else if(param->kind == A_varDeclType::A_varDeclArrayKind){
            aA_varDeclArray arr = param->u.declArray;
            tc_type rv_type = get_rval_type(out, rv);
            if(!comp_tc_type(param_type, rv_type)){
                string error_message = errmsg_fnCall_param_check(func_name, fc->pos);
                error_print(out, fc->pos, error_message);
            }
            if (rv->kind == A_rightValType::A_arithExprValKind &&
                    rv->u.arithExpr->kind == A_arithExprType::A_exprUnitKind &&
                    rv->u.arithExpr->u.exprUnit->kind == A_exprUnitType::A_idExprKind){
                string name = *rv->u.arithExpr->u.exprUnit->u.id;
                aA_varDeclArray arr = param->u.declArray;
                int len = get_array_len(name);
                if (len != arr->len || !comp_tc_type(get_array_type(name), param_type)){
                    string error_message = errmsg_fnCall_param_check(func_name, fc->pos);
                    error_print(out, fc->pos, error_message);
                }
            }
        }else{
            string error_message = errmsg_fnCall_param_check(func_name, fc->pos);
            error_print(out, fc->pos, error_message);
        }
    }
    return;
}


void check_WhileStmt(std::ostream& out, aA_whileStmt ws){
    if(!ws)
        return;
    check_BoolExpr(out, ws->boolExpr);
    /* fill code here, take care of variable scope */
    local_token2Type.push_back(new typeMap());
        
    for(aA_codeBlockStmt s : ws->whileStmts){
        check_CodeblockStmt(out, s);
    }
    /* fill code here */
    local_token2Type.pop_back();
        
    return;
}


void check_CallStmt(std::ostream& out, aA_callStmt cs){
    if(!cs)
        return;
    check_FuncCall(out, cs->fnCall);
    return;
}


void check_ReturnStmt(std::ostream& out, aA_returnStmt rs){
    if(!rs)
        return;
    tc_type should = g_token2Type[current_func];
    tc_type is = get_rval_type(out, rs->retVal);
    if(!comp_aA_type(should->type, is->type)){
        error_print(out, rs->pos, "Return type mismatch!");
    }
return;
}

