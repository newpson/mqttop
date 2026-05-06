#pragma once
#include "mt_tags.h"
#include <glib.h>

typedef GArray MT_Equation;

typedef enum {
    MT_EquationBool_False = false,
    MT_EquationBool_True = true,
    MT_EquationBool_NONE = -1,
} MT_EquationBool;

/// Компилирует выражение, заданное строкой `string`, добавляя все найденные в ней переменные в `variables`.  
/// Возвращает `NULL`, если при компиляции возникла ошибка (на этом этапе можно словить лишь синтаксические ошибки).
MT_Equation *mt_equation_compile(const char *const string, MT_TagStorage *const variables);

/// Освобождает память, занятую `equation`.
void mt_equation_free(MT_Equation *const equation);

/// Вычисляет выражение, подставляя `EquationBool_True` вместо переменных, содержащихся в `true_variables`.
/// Вместо переменных, которых нет в `true_variables`, подставляется `EquationBool_False`.
///
/// Если при вычислении возникла ошибка, то возвращает `EquationBool_None`.
///
/// `true_variables` может быть `NULL`. В таком случае будет лишь проверена семантическая корректность выражения
/// (вместо всех переменных будет подставлено `EquationBool_False`).
MT_EquationBool mt_equation_evaluate(const MT_Equation *const equation, const MT_TagSet *const true_variables);