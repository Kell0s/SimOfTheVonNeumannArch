/*
 * Принципы архитектуры фон Неймана:
 * 1) У программы общая память:
 *    один массив memory[256], где хранятся данные и команды.
 * 2) Выполнение происходит последовательно:
 *    команды выполняются по порядку.
 *    cpu_state[0] += 2, после выполнения идет след команда
 * 3) Номер команд:
 *    Процессор знает, где он находится только по pc (cpu_state[0]).
 *    Для изменения порядка нужно переписать pc.
 * 4) Поцессор это регистры и АЛУ:
 *    Внутри процессора регистры (cpu_state[8]), switch - АЛУ
 * Итог:
 * Программа - это числа в общей памяти, процессор достает
 * каждое число по одному по счетчику команд и выполняет.
 */
#include <stdio.h>

int main(void) {
    // База ошибок
    const char *errors[] = {
        "нет ошибки",
        "адрес в неверном диапозоне",
        "неизвестная команда",
        "счетчик команд в неверном диапозоне",
        "неверный вектор прерываний",
    };

    int cpu_state[8] = {0};
    int memory[256] = {0};
    int x;

    // Открываем файл
    FILE *f = fopen("program.txt", "r");
    if (f == NULL) {
        printf("ошибка: не нашли файл\n");
        return 1;
    }

    // Передаем в memory числа
    int i = 0;
    while (fscanf(f, "%d", &x) == 1) {
        memory[i] = x;
        i++;
    }
    fclose(f);

    // проверка вектора прерываний. Выходит ли адрес за память
    if (memory[0] < 0 || memory[0] > 254) {
        printf("err: %s\n", errors[4]);
        return 1;
    }

    // Ввод адреса первой команды
    int start;
    printf("Адрес первой комманды:");
    scanf("%d", &start);

    // Задаем первую команду и вектор прерываний
    cpu_state[0] = start;
    cpu_state[3] = memory[0];

    // Начало программы
    while (cpu_state[4] == 0) {
        // берем pc
        int pc = cpu_state[0];

        // Проверяем входил ли он в память
        if (pc < 0 || pc > 254) {
            cpu_state[5] = 3;
            printf("err: %s\n", errors[3]);

            /*
             * Прерывание: при ошибке прыгаем на обработчик (адрес из memory[0]).
             * При cpu_state[6] флаг "уже в обработчике". Если ошибка повторилась
             * внутри обработчика останавливаем машину
             */
            if (cpu_state[6] == 1) {
                printf("повторная ошибка\n");
                cpu_state[4] = 1;
                break;
            }

            cpu_state[6] = 1;
            cpu_state[0] = cpu_state[3];
            continue;
        }

        // Задаем регистр команды, если 0, то конец программы
        int cmd = memory[pc];
        cpu_state[2] = cmd;

        if (cmd == 0) {
            cpu_state[4] = 1;
            printf("pc=%d, cmd=0, STOP\n", pc);
            break;
        }

        // Задаем адрес числа, для будущей работы с ним
        int addr = memory[pc + 1];

        // Проверка входит ли адрес в память
        if (addr < 0 || addr > 255) {
            cpu_state[5] = 1;
            printf("pc=%d, cmd=%d, operand=%d, err: %s\n", pc, cmd, addr, errors[1]);

            if (cpu_state[6] == 1) {
                printf("повторная ошибка\n");
                cpu_state[4] = 1;
                break;
            }

            cpu_state[6] = 1;
            cpu_state[0] = cpu_state[3];
            continue;
        }

        // Команды
        switch (cmd) {
        case 1:
            // Загрузить в акк
            cpu_state[1] = memory[addr];
            cpu_state[0] += 2;
            break;
        case 2:
            // Прибавить к акк
            cpu_state[1] += memory[addr];
            cpu_state[0] += 2;
            break;
        case 3:
            // Вычесть из акк
            cpu_state[1] -= memory[addr];
            cpu_state[0] += 2;
            break;
        case 4:
            // Безусловный переход (JUMP)
            cpu_state[0] = addr;
            break;
        case 5:
            // условный перехож (JUMP IF ZERO)
            if (cpu_state[1] == 0) {
                cpu_state[0] = addr;
            } else {
                cpu_state[0] += 2;
            }
            break;
        default:
            // Неизвестная команда
            cpu_state[5] = 2;
            printf("pc=%d, cmd=%d, operand=%d, err: %s\n", pc, cmd, addr, errors[2]);

            if (cpu_state[6] == 1) {
                printf("повторная ошибка\n");
                cpu_state[4] = 1;
                continue;
            }

            cpu_state[6] = 1;
            cpu_state[0] = cpu_state[3];
            continue;
        }

        printf("pc=%d, cmd=%d, opperand=%d, acc=%d\n", pc, cmd, addr, cpu_state[1]);
    }

    // Вывод ошибки или успеха
    if (cpu_state[5] != 0) {
        printf("ошибка %d: %s\n", cpu_state[5], errors[cpu_state[5]]);
    } else {
        printf("успешно!\n");
    }

    return 0;
}
