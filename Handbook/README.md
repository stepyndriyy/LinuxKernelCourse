# Handbook

---
Взаимодействие с девайсом

- добавление пользователя
    ```sh
    echo "add <фамилия> <номер> <email>" > /dev/handbook
    ```
- удаление пользователя
    ```sh
    echo "del <фамилия>" > /dev/handbook
    ```
- запрос на получение данных
    ```sh
    echo "get <фамилия>" > /dev/handbook
    ```
- получение данных запроса
    ```sh
    head -c20 /dev/handbook
    ```
---
## Как пользоваться

1) make
2) sudo insmod handbook.ko
3) ./example.sh
4) dmesg - проверяем корректность

