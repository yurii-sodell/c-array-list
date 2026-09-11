

#ifndef ARRAY_LIST_T_LIB_MESSAGES_HANDLER_H
#define ARRAY_LIST_T_LIB_MESSAGES_HANDLER_H

#include "../array_list_t.h"

void arr_handle_internal_operation_status(arr_status st, char* source);
char* arr_build_error_message_for_custom_types(char* source, char* type1, char* type2);
void arr_handle_error_message_without_status(char* message);

#endif //ARRAY_LIST_T_LIB_MESSAGES_HANDLER_H