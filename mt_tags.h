#pragma once
#include <stddef.h>
#include <glib.h>

typedef struct MT_TagStorage MT_TagStorage;
typedef GHashTable MT_TagSet;

/// Создаёт новое фрагментированное хэш-хранилище нуль-терминированных строк с заданным размером блока (в байтах).
/// Указатели строк не изменяют своего значения при автоматическом изменении размеров хранилища.
MT_TagStorage *mt_tag_storage_new(const size_t block_size_bytes);

/// Освобождает память, занятую хранилищем.
void mt_tag_storage_destroy(MT_TagStorage *const tags);

/// Копирует новую строку `string` в хранилище, расширяя его, если нужно.
///
/// Возвращает перманентный указатель на эту строку в хранилище.
/// Если строка уже есть в хранилище, то возращается её перманентный адрес.
const char *mt_tag_storage_push(MT_TagStorage *const tags, const char *const string);

/// Возвращает перманентный указатель на строку `string` в хранилище.
/// Если строка отсутствует в хранилище, то возращается `NULL`.
const char *mt_tag_storage_lookup(const MT_TagStorage *const tags, const char *const string);

/// Создаёт новую хэш-таблицу указателей (быстрая версия хранилища для уже существующих строк)
/// Хэш-функция применяется напрямую к адресу указателя на строку, за счёт чего и обеспечивается быстродейтствие.
MT_TagSet *mt_tag_set_new();
void mt_tag_set_destroy(MT_TagSet *const tags);
void mt_tag_set_push(MT_TagSet *const tags, const char *const string);
const char *mt_tag_set_lookup(const MT_TagSet *const tags, const char *const string);
void mt_tag_set_clear(MT_TagSet *const tags);