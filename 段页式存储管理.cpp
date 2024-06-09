#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SEGMENTS 1000
#define MAX_PAGES 2048

int **table;  // 位示图表
int MemorySize;   // 主存大小
int BlockNum;     // 块数
int BlockLength;  // 块长
int WordLength;   // 字长
int WordNum;      // 字数

// 页表
typedef struct ye {
    int num;    // 页号
    int block;  // 块号
} ye;

// 段表
typedef struct duan {
    int duan_num;  // 段号
    char state[10];// 状态
    int y_len;     // 页长
    ye *B;         // 页表
} duan;

// 作业
typedef struct work {
    int size;          // 作业大小
    char name[20];     // 作业名字
    int d_len;         // 段数
    duan *A;           // 段表
    struct work *next; // 下一个作业
} work;

work* head = NULL; // 作业链表头

// 初始化位示图
void init_bitmap() {
    for (int i = 0; i < BlockNum; i++) {
        for (int j = 0; j < BlockLength; j++) {
            table[i][j] = 0;
        }
    }
}

// 查找空闲块
int find_free_block() {
    for (int i = 0; i < BlockNum; i++) {
        for (int j = 0; j < BlockLength; j++) {
            if (table[i][j] == 0) {
                return i * BlockLength + j;
            }
        }
    }
    return -1; // 无空闲块
}

// 分配物理块
void allocate_blocks(work *new_work) {
    for (int i = 0; i < new_work->d_len; i++) {
        duan *segment = &new_work->A[i];
        for (int j = 0; j < segment->y_len; j++) {
            int block = find_free_block();
            if (block == -1) {
                printf("内存不足，无法分配\n");
                return;
            }
            table[block / BlockLength][block % BlockLength] = 1; // 标记为已分配
            segment->B[j].num = j;
            segment->B[j].block = block;
        }
    }
}

// 释放物理块
void release_blocks(work *target_work) {
    for (int i = 0; i < target_work->d_len; i++) {
        duan *segment = &target_work->A[i];
        for (int j = 0; j < segment->y_len; j++) {
            int block = segment->B[j].block;
            table[block / BlockLength][block % BlockLength] = 0; // 标记为未分配
        }
    }
}

// 动态重定位地址转换
int translate_address(work *target_work, int segment_num, int page_num, int offset) {
    if (segment_num >= target_work->d_len || page_num >= target_work->A[segment_num].y_len) {
        printf("无效的段或页\n");
        return -1;
    }
    int block = target_work->A[segment_num].B[page_num].block;
    return block * BlockLength + offset;
}

// 添加作业
void add_work(char *name, int size, int d_len, int y_len[]) {
    work *new_work = (work *)malloc(sizeof(work));
    strcpy(new_work->name, name);
    new_work->size = size;
    new_work->d_len = d_len;
    new_work->A = (duan *)malloc(d_len * sizeof(duan));
    for (int i = 0; i < d_len; i++) {
        new_work->A[i].duan_num = i;
        strcpy(new_work->A[i].state, "allocated");
        new_work->A[i].y_len = y_len[i];
        new_work->A[i].B = (ye *)malloc(y_len[i] * sizeof(ye));
    }
    allocate_blocks(new_work);

    new_work->next = head;
    head = new_work;

    // 计算并显示剩余内存大小
    int used_blocks = 0;
    for (int i = 0; i < BlockNum; i++) {
        for (int j = 0; j < BlockLength; j++) {
            if (table[i][j] == 1) {
                used_blocks++;
            }
        }
    }
    int remaining_memory = MemorySize - used_blocks * WordLength;
    printf("剩余内存大小: %d\n", remaining_memory);
}

// 释放作业
void remove_work(char *name) {
    work *current = head;
    work *prev = NULL;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            if (prev == NULL) {
                head = current->next;
            } else {
                prev->next = current->next;
            }
            release_blocks(current);
            free(current);
            printf("作业 %s 已释放\n", name);

            // 计算并显示剩余内存大小
            int used_blocks = 0;
            for (int i = 0; i < BlockNum; i++) {
                for (int j = 0; j < BlockLength; j++) {
                    if (table[i][j] == 1) {
                        used_blocks++;
                    }
                }
            }
            int remaining_memory = MemorySize - used_blocks * WordLength;
            printf("剩余内存大小: %d\n", remaining_memory);

            return;
        }
        prev = current;
        current = current->next;
    }
    printf("未找到作业 %s\n", name);
}

// 打印位示图
void print_bitmap() {
    printf("位示图:\n");
    for (int i = 0; i < BlockNum; i++) {
        for (int j = 0; j < BlockLength; j++) {
            printf("%d ", table[i][j]);
        }
        printf("\n");
    }
}

// 打印作业信息
void print_work_info() {
    work *current = head;
    while (current != NULL) {
        printf("作业名: %s, 大小: %d, 段数: %d\n", current->name, current->size, current->d_len);
        for (int i = 0; i < current->d_len; i++) {
            printf(" 段号: %d, 页长: %d\n", current->A[i].duan_num, current->A[i].y_len);
            for (int j = 0; j < current->A[i].y_len; j++) {
                printf("  页号: %d, 块号: %d\n", current->A[i].B[j].num, current->A[i].B[j].block);
            }
        }
        current = current->next;
    }
}

int main() {
    // 初始化
    printf("请输入内存大小: ");
    scanf("%d", &MemorySize);
    printf("请输入系统字长大小: ");
    scanf("%d", &WordLength);
    printf("请输入块长大小: ");
    scanf("%d", &BlockLength);

    BlockNum = MemorySize / (BlockLength * WordLength);

    // 动态分配位示图
    table = (int **)malloc(BlockNum * sizeof(int *));
    for (int i = 0; i < BlockNum; i++) {
        table[i] = (int *)malloc(BlockLength * sizeof(int));
    }

    init_bitmap();

    char name[20];
    int size, d_len;

    while (1) {
        printf("请输入作业名称 (输入 'exit' 退出): ");
        scanf("%s", name);
        if (strcmp(name, "exit") == 0) break;

        printf("请输入作业大小: ");
        scanf("%d", &size);
        printf("请输入作业的段数: ");
        scanf("%d", &d_len);

        int y_len[d_len];
        for (int i = 0; i < d_len; i++) {
            printf("请输入第 %d 段的页数: ", i);
            scanf("%d", &y_len[i]);
        }

        add_work(name, size, d_len, y_len);

        printf("位示图:\n");
        print_bitmap();

        printf("作业信息:\n");
        print_work_info();

        printf("请输入要释放的作业名称 (输入 'none' 跳过): ");
        scanf("%s", name);
        if (strcmp(name, "none") != 0) {
            remove_work(name);
        }

        printf("位示图:\n");
        print_bitmap();
    }

    // 释放位示图内存
    for (int i = 0; i < BlockNum; i++) {
        free(table[i]);
    }
    free(table);

    return 0;
}
