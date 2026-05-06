#include "mt_equation.h"
#include "mt_logf.h"
#include "mt_tags.h"
#include <ctype.h>
#include <glib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

typedef enum {
    MT_EquationOperator_Begin,
    MT_EquationOperator_End,
    MT_EquationOperator_Not,
    MT_EquationOperator_And,
    MT_EquationOperator_Or,
    MT_EquationOperator_COUNT,
    MT_EquationOperator_NONE = -1,
} MT_EquationOperator;

typedef GArray MT_ArgStack;

static inline MT_ArgStack *mt_arg_stack_new()
{
    return g_array_new(FALSE, FALSE, sizeof(MT_EquationBool));
}

static inline void mt_arg_stack_free(MT_ArgStack *const args)
{
    g_array_free(args, TRUE);
}

static inline void mt_arg_stack_push(MT_ArgStack *const args, const MT_EquationBool value)
{
    g_array_append_val(args, value);
}

static MT_EquationBool mt_arg_stack_top(MT_ArgStack *const args)
{
    if (args->len == 0)
        return MT_EquationBool_NONE;
    return g_array_index(args, MT_EquationBool, args->len - 1);
}

static MT_EquationBool mt_arg_stack_pull(MT_ArgStack *const args)
{
    if (args->len == 0)
        return MT_EquationBool_NONE;
    g_array_set_size(args, args->len - 1);
    return g_array_index(args, MT_EquationBool, args->len);
}

static inline size_t mt_arg_stack_size(const MT_ArgStack *const args)
{
    return args->len;
}

typedef GArray MT_OpStack;

static inline MT_OpStack *mt_op_stack_new()
{
    return g_array_new(FALSE, FALSE, sizeof(MT_EquationOperator));
}

static inline void mt_op_stack_free(MT_OpStack *const ops)
{
    g_array_free(ops, TRUE);
}

static inline void mt_op_stack_push(MT_OpStack *const ops, const MT_EquationOperator value)
{
    g_array_append_val(ops, value);
}

static MT_EquationOperator mt_op_stack_top(MT_OpStack *const ops)
{
    if (ops->len == 0)
        return MT_EquationOperator_NONE;
    return g_array_index(ops, MT_EquationOperator, ops->len - 1);
}

static void mt_op_stack_pop(MT_OpStack *const ops)
{
    if (ops->len == 0)
        return;
    g_array_set_size(ops, ops->len - 1);
}

static MT_EquationOperator mt_op_stack_pull(MT_OpStack *const ops)
{
    if (ops->len == 0)
        return MT_EquationOperator_NONE;
    g_array_set_size(ops, ops->len - 1);
    return g_array_index(ops, MT_EquationOperator, ops->len);
}

static inline size_t mt_op_stack_size(const MT_OpStack *const ops)
{
    return ops->len;
}

typedef void (*MT_EquationOperatorFunction_FunPtr)(MT_ArgStack *const args, MT_OpStack *const ops);

void mt_equation_operator_function_begin(MT_ArgStack *const, MT_OpStack *const);
void mt_equation_operator_function_end(MT_ArgStack *const, MT_OpStack *const);
void mt_equation_operator_function_not(MT_ArgStack *const, MT_OpStack *const);
void mt_equation_operator_function_and(MT_ArgStack *const, MT_OpStack *const);
void mt_equation_operator_function_or(MT_ArgStack *const, MT_OpStack *const);

typedef struct {
    char repr;
    size_t priority;
    MT_EquationOperatorFunction_FunPtr eval;
} EquationOperatorInfo;

static const EquationOperatorInfo EquationOperators[MT_EquationOperator_COUNT] = {
    [MT_EquationOperator_Begin] = { '(', 3, mt_equation_operator_function_begin },
    [MT_EquationOperator_End] = { ')', 3, mt_equation_operator_function_end },
    [MT_EquationOperator_Not] = { '!', 2, mt_equation_operator_function_not },
    [MT_EquationOperator_And] = { '&', 1, mt_equation_operator_function_and },
    [MT_EquationOperator_Or] = { '|', 0, mt_equation_operator_function_or },
};

static bool mt_is_equation_operator(const char c)
{
    for (MT_EquationOperator op = MT_EquationOperator_Begin; op < MT_EquationOperator_COUNT; ++op)
        if (c == EquationOperators[op].repr)
            return true;
    return false;
}

void mt_equation_operator_function_begin(MT_ArgStack *const, MT_OpStack *const)
{
    return;
}

void mt_equation_operator_function_end(MT_ArgStack *const args, MT_OpStack *const ops)
{
    MT_EquationOperator op = mt_op_stack_top(ops);
    while (op != MT_EquationOperator_Begin) {
        if (op == MT_EquationOperator_NONE) {
            g_warning("unmatched \"(\"\n");
            mt_arg_stack_push(args, MT_EquationBool_NONE);
            return;
        }
        EquationOperators[op].eval(args, ops);
        mt_op_stack_pop(ops);
        op = mt_op_stack_top(ops);
    }
    mt_op_stack_pop(ops); // EquationOperator_Begin
}

void mt_equation_operator_function_and(MT_ArgStack *const args, MT_OpStack *const)
{
    MT_EquationBool arg_right = mt_arg_stack_pull(args);
    MT_EquationBool arg_left = mt_arg_stack_pull(args);
    if (arg_right != MT_EquationBool_NONE && arg_left != MT_EquationBool_NONE) {
        mt_arg_stack_push(args, arg_left && arg_right);
        return;
    }
    g_log(__FILE__, G_LOG_LEVEL_WARNING, "EquationBool_NONE\n");
    mt_arg_stack_push(args, MT_EquationBool_NONE);
}

void mt_equation_operator_function_not(MT_ArgStack *const args, MT_OpStack *const)
{
    MT_EquationBool arg = mt_arg_stack_pull(args);
    if (arg != MT_EquationBool_NONE) {
        mt_arg_stack_push(args, !arg);
        return;
    }
    mt_warnf("EquationBool_NONE in argument stack");
    mt_arg_stack_push(args, MT_EquationBool_NONE);
}

void mt_equation_operator_function_or(MT_ArgStack *const args, MT_OpStack *const)
{
    MT_EquationBool arg_right = mt_arg_stack_pull(args);
    MT_EquationBool arg_left = mt_arg_stack_pull(args);
    if (arg_right != MT_EquationBool_NONE && arg_left != MT_EquationBool_NONE) {
        mt_arg_stack_push(args, arg_left || arg_right);
        return;
    }
    mt_warnf("EquationBool_NONE in argument stack");
    mt_arg_stack_push(args, MT_EquationBool_NONE);
}

typedef enum {
    EquationTokenType_Variable,
    EquationTokenType_Operator,
} MT_EquationTokenType;

typedef struct {
    MT_EquationTokenType type;
    union {
        const char *variable;
        MT_EquationOperator operator;
    };
} MT_EquationToken;

static void mt_equation_push_variable(MT_Equation *const equation, const char *const variable_name, MT_TagStorage *const tags)
{
    const char *const persistent_string = mt_tag_storage_push(tags, variable_name);
    const MT_EquationToken token = {
        .type = EquationTokenType_Variable,
        .variable = persistent_string
    };
    g_array_append_val(equation, token);
}

static inline void mt_equation_push_operator(MT_Equation *const equation, const MT_EquationOperator operator)
{
    const MT_EquationToken token = {
        .type = EquationTokenType_Operator,
        .operator = operator
    };
    g_array_append_val(equation, token);
}

typedef enum {
    MT_EquationEscape_B,
    MT_EquationEscape_F,
    MT_EquationEscape_N,
    MT_EquationEscape_R,
    MT_EquationEscape_T,
    MT_EquationEscapeCode_COUNT,
} MT_EquationEscape;

typedef struct {
    char code;
    char replacement;
} MT_EquationEscapeInfo;

static const MT_EquationEscapeInfo EquationEscapes[MT_EquationEscapeCode_COUNT] = {
    [MT_EquationEscape_B] = { 'b', '\b' },
    [MT_EquationEscape_F] = { 'f', '\f' },
    [MT_EquationEscape_N] = { 'n', '\n' },
    [MT_EquationEscape_R] = { 'r', '\r' },
    [MT_EquationEscape_T] = { 't', '\t' },
};

static const char *mt_equation_extract_varname_quoted(char *varname_iter, const char *string_iter)
{
    while (*string_iter != '\0' && *string_iter != '"') {
        MT_EquationEscape ec = MT_EquationEscapeCode_COUNT;
        if (*string_iter == '\\') {
            string_iter += 1;
            if (*string_iter == '\0') {
                mt_errorf("Trailing backslash ('\\')");
                return NULL;
            }
            for (ec = MT_EquationEscape_B; ec < MT_EquationEscapeCode_COUNT; ++ec) {
                if (*string_iter == EquationEscapes[ec].code) {
                    *varname_iter = EquationEscapes[ec].replacement;
                    break;
                }
            }
        }
        const bool is_escaped = (ec < MT_EquationEscapeCode_COUNT);
        if (!is_escaped)
            *varname_iter = *string_iter;
        string_iter += 1;
        varname_iter += 1;
    }
    *varname_iter = '\0';
    if (*string_iter != '"') {
        mt_errorf("Unclosed quote ('\"')");
        return NULL;
    }
    return string_iter;
}

static const char *mt_extract_varname(char *varname, const char *string_iter)
{
    while (*string_iter != '\0' && *string_iter != '\"' && !isspace(*string_iter) && !mt_is_equation_operator(*string_iter)) {
        *varname = *string_iter;
        varname += 1;
        string_iter += 1;
    }
    *varname = '\0';
    return string_iter - 1; // for loop increment correction
}

MT_Equation *mt_equation_compile(const char *const string, MT_TagStorage *const tags)
{
    MT_Equation *equation = g_array_new(FALSE, FALSE, sizeof(MT_EquationToken));
    char *const varname = g_malloc((strlen(string) + 1) * sizeof(char));

    mt_equation_push_operator(equation, MT_EquationOperator_Begin);
    for (const char *string_iter = string; *string_iter != '\0'; ++string_iter) {
        MT_EquationOperator op;
        for (op = MT_EquationOperator_Begin; op < MT_EquationOperator_COUNT; ++op) {
            if (*string_iter == EquationOperators[op].repr) {
                mt_equation_push_operator(equation, op);
                goto outer_continue;
            }
        }
        if (isspace(*string_iter))
            continue;
        if (*string_iter == '"') {
            string_iter += 1;
            string_iter = mt_equation_extract_varname_quoted(varname, string_iter);
        } else {
            string_iter = mt_extract_varname(varname, string_iter);
        }
        if (string_iter == NULL) {
            mt_errorf("Variable name extraction error");
            goto free_all;
        }
        mt_equation_push_variable(equation, varname, tags);
    outer_continue:
    }
    mt_equation_push_operator(equation, MT_EquationOperator_End);

    g_free(varname);
    return equation;

free_all:
    g_free(varname);
    mt_equation_free(equation);
    return NULL;
}

MT_EquationBool mt_equation_evaluate(const MT_Equation *const equation, const MT_TagSet *const tags)
{
    MT_ArgStack *args = mt_arg_stack_new();
    MT_OpStack *ops = mt_op_stack_new();

    for (size_t i = 0; i < equation->len; ++i) {
        const MT_EquationToken *token = &g_array_index(equation, MT_EquationToken, i);
        if (token->type == EquationTokenType_Variable) {
            const bool is_present = (mt_tag_set_lookup(tags, token->variable) != NULL);
            mt_arg_stack_push(args, (MT_EquationBool)is_present);
        } else { // token->type == EquationTokenType_Operator
            const MT_EquationOperator cur_op = token->operator;
            switch (cur_op) {
            case MT_EquationOperator_Begin:
                mt_op_stack_push(ops, cur_op);
                break;
            case MT_EquationOperator_End:
                EquationOperators[cur_op].eval(args, ops);
                break;
            default:
                MT_EquationOperator top_op = mt_op_stack_top(ops);
                while (top_op != MT_EquationOperator_NONE
                    && top_op != MT_EquationOperator_Begin
                    && EquationOperators[top_op].priority >= EquationOperators[cur_op].priority) {
                    EquationOperators[top_op].eval(args, ops);
                    mt_op_stack_pop(ops);
                    top_op = mt_op_stack_top(ops);
                }
                mt_op_stack_push(ops, cur_op);
                break;
            }
        }
    }

    MT_EquationBool result;
    if (mt_op_stack_size(ops) > 0) {
        mt_errorf("Redundant operators");
        result = MT_EquationBool_NONE;
    } else if (mt_arg_stack_size(args) != 1) {
        mt_errorf("Redundant arguments");
        result = MT_EquationBool_NONE;
    } else {
        result = mt_arg_stack_top(args);
    }

    mt_op_stack_free(ops);
    mt_arg_stack_free(args);
    return result;
}

void mt_equation_free(MT_Equation *const equation)
{
    g_array_free(equation, TRUE);
}