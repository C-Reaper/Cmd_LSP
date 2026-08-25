#include "/home/codeleaded/System/Static/Library/LSP.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* Read_File(const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file) return NULL;

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }
    long size = ftell(file);
    if (size < 0) {
        fclose(file);
        return NULL;
    }
    rewind(file);

    char* data = malloc((size_t)size + 1U);
    if (!data) {
        fclose(file);
        return NULL;
    }

    size_t read = fread(data, 1, (size_t)size, file);
    fclose(file);
    data[read] = '\0';
    return data;
}
static void Print_Message(const char* title, LSP_Package* package) {
    LSP_MessageInfo info = {0};
    LSP_Message_Inspect(package, &info);

    printf("\n================ %s ================\n", title);

    if (info.type == LSP_MESSAGE_RESPONSE) {
        printf("type: response\n");
        printf("id: %llu\n", (unsigned long long)info.id);
        printf("error: %s\n", info.error ? "yes" : "no");
    } else if (info.type == LSP_MESSAGE_NOTIFICATION) {
        printf("type: notification\n");
        printf("method: %s\n", info.method ? info.method : "?");
    } else if (info.type == LSP_MESSAGE_REQUEST) {
        printf("type: request\n");
        printf("id: %llu\n", (unsigned long long)info.id);
        printf("method: %s\n", info.method ? info.method : "?");
    } else {
        printf("type: invalid/unknown\n");
    }

    printf("%s\n", package->data ? package->data : "");
    printf("========================================\n");

    LSP_MessageInfo_Free(&info);
}
static char Wait_Print_Response(LSP* lsp, LSP_RequestID id, const char* title) {
    LSP_Package package = LSP_Package_New();

    if (!LSP_Wait_Response(lsp, id, &package, 5000U)) {
        fprintf(stderr, "Timeout waiting for response %llu (%s).\n", (unsigned long long)id, title);
        return 0;
    }

    Print_Message(title, &package);
    LSP_Package_Free(&package);
    return 1;
}
static void Drain_Notifications(LSP* lsp, unsigned int milliseconds) {
    uint64_t start = 0;
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    start = (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)ts.tv_nsec / 1000000ULL;

    while (1) {
        LSP_Package package = LSP_Package_New();

        if (LSP_Poll(lsp, &package)) {
            Print_Message("NOTIFICATION / SERVER MESSAGE", &package);
            LSP_Package_Free(&package);
            continue;
        }

        timespec_get(&ts, TIME_UTC);
        uint64_t now = (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)ts.tv_nsec / 1000000ULL;
        if (now - start >= milliseconds) break;

        Thread_Sleep_M(1);
    }
}

int main(void) {
    const char* clangd_args[] = {"clangd", "--background-index", NULL};
    const char* project = "/home/codeleaded/Hecke/C/Cmd_LSP";
    const char* source = "/home/codeleaded/Hecke/C/Cmd_LSP/code/test.c";
    const char* uri = "file:///home/codeleaded/Hecke/C/Cmd_LSP/code/test.c";
    const char* root_uri = "file:///home/codeleaded/Hecke/C/Cmd_LSP";

    LSP_Language clangd = LSP_Language_New("c", "clangd", clangd_args);
    LSP lsp = LSP_New();

    if (!LSP_Connect(&lsp, &clangd, project)) {
        fprintf(stderr, "Failed to start clangd.\n");
        LSP_Free(&lsp);
        return 1;
    }

    LSP_RequestID initialize_id = LSP_Initialize(&lsp, root_uri);

    if (!initialize_id) {
        fprintf(stderr, "Could not send initialize.\n");
        LSP_Free(&lsp);
        return 1;
    }
    if (!Wait_Print_Response(&lsp, initialize_id, "INITIALIZE")) {
        LSP_Free(&lsp);
        return 1;
    }
    if (!LSP_Initialized(&lsp)) {
        fprintf(stderr, "Could not send initialized notification.\n");
        LSP_Free(&lsp);
        return 1;
    }

    char* source_text = Read_File(source);
    if (!source_text) {
        fprintf(stderr, "Could not read: %s\n", source);
        LSP_Shutdown(&lsp);
        LSP_Free(&lsp);
        return 1;
    }
    if (!LSP_DidOpen(&lsp, uri, "c", 1, source_text)) {
        fprintf(stderr, "Could not send didOpen.\n");
        free(source_text);
        LSP_Shutdown(&lsp);
        LSP_Free(&lsp);
        return 1;
    }

    LSP_Package diagnostics = LSP_Package_New();
    if (LSP_Wait_Notification(&lsp, "textDocument/publishDiagnostics", &diagnostics, 3000U)) {
        Print_Message("INITIAL DIAGNOSTICS", &diagnostics);
        LSP_Package_Free(&diagnostics);
    } else {
        printf("No diagnostics notification received within 3 seconds.\n");
    }


    const int line = 2;
    const int character = 4;

    LSP_RequestID hover_id = LSP_Hover(&lsp, uri, line, character);

    if (hover_id) Wait_Print_Response(&lsp, hover_id, "HOVER");

    LSP_RequestID completion_id = LSP_Completion(&lsp, uri, line, character);

    if (completion_id) Wait_Print_Response(&lsp, completion_id, "COMPLETION");

    LSP_RequestID definition_id = LSP_Definition(&lsp, uri, line, character);

    if (definition_id) Wait_Print_Response(&lsp, definition_id, "DEFINITION");

    LSP_RequestID declaration_id = LSP_Declaration(&lsp, uri, line, character);

    if (declaration_id) Wait_Print_Response(&lsp, declaration_id, "DECLARATION");

    LSP_RequestID references_id = LSP_References(&lsp, uri, line, character);

    if (references_id) Wait_Print_Response(&lsp, references_id, "REFERENCES");

    LSP_RequestID symbols_id = LSP_DocumentSymbols(&lsp, uri);

    if (symbols_id) Wait_Print_Response(&lsp, symbols_id, "DOCUMENT SYMBOLS");

    LSP_RequestID signature_id = LSP_SignatureHelp(&lsp, uri, line, character);

    if (signature_id) Wait_Print_Response(&lsp, signature_id, "SIGNATURE HELP");

    const char* modified_text = "#include <stdio.h>\n"
                                "\n"
                                "int main(void)\n"
                                "{\n"
                                "    printf(\"Hello from LSP\\n\");\n"
                                "    does_not_exist();\n"
                                "    return 0;\n"
                                "}\n";

    if (!LSP_DidChange(&lsp, uri, 2, modified_text)) {
        fprintf(stderr, "Could not send didChange.\n");
    }

    if (LSP_Wait_Notification(&lsp, "textDocument/publishDiagnostics", &diagnostics, 3000U)) {
        Print_Message("DIAGNOSTICS AFTER CHANGE", &diagnostics);
        LSP_Package_Free(&diagnostics);
    }

    LSP_TextEdit edit;
    edit.range.start.line = 5;
    edit.range.start.character = 4;
    edit.range.end.line = 5;
    edit.range.end.character = 20;
    edit.text = "does_not_exist";


    LSP_DidClose(&lsp, uri);
    free(source_text);

    LSP_RequestID shutdown_id = LSP_Shutdown(&lsp);

    if (shutdown_id) Wait_Print_Response(&lsp, shutdown_id, "SHUTDOWN");

    LSP_Exit(&lsp);
    LSP_Free(&lsp);
    return 0;
}