#pragma once

typedef struct MT_EntryList MT_EntryList;

/// Начинает новую entry.
void mt_entry_begin(MT_EntryList *const entry_list, const char *const id);

/// Добавляет новый тег к записи (если ещё не присвоен).
void mt_entry_push_tag(MT_EntryList *const entry_list, const char *const tag);

/// Отправляет текущий набор тегов записи на проверку.
/// Возвращает результат проверки: если запись будет добавлена в результирующий json-пакет, возвращается `true`.
bool mt_entry_is_suitable(MT_EntryList *const entry_list);

/// Отменяет работу с данной записью.
/// Если `MQTTOP_EntryOffer` вернул `false`, то есть смысл вызвать `MQTTOP_EntryEnd` сразу, не устанавливая значение.
/// Если `MQTTOP_EntryOffer` не вызывался для записи, то будет вызван автоматически.
// void mt_entry_discard(MT_Entry *const entry);

/// Устанавливает значение и тип записи, завершая работу с записью
void mt_entry_set_value_null(MT_EntryList *const entry_list);
void mt_entry_set_value_bool(MT_EntryList *const entry_list, const bool value);
void mt_entry_set_value_number(MT_EntryList *const entry_list, const double value);
void mt_entry_set_value_string(MT_EntryList *const entry_list, const char *const value);