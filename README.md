# Архиватор Хаффмана на языке Си

Архиватор Хаффмана — это программа, которая сжимает файлы, используя алгоритм Хаффмана. Алгоритм Хаффмана — это метод сжатия без потерь, который работает путём создания дерева, где более часто встречающиеся символы имеют более короткие коды.

Моя реализация работает в основном с текстовыми файлами.

## Установка

После клонирования этого репозитория, необходимо выполнить следующие команды
```
cd huffman_project
make
```
После этого будет создан исполняемый файл huffman, который и будет в дальнейшем использоваться

## Использование

Для использования написанной программы необходимо пользоваться следующим интерфейсом:

```
./huffman -e/-d <input> <output>
```

Для того, чтобы закодировать <input> необходимо передать аргумент -e.
Для его декодирования передается аргумент -d.

## Пример использования

Для проверки архиватора можно запустить его на файле message.txt следующим образом:

```
./huffman -e message.txt coded_message.bin
```

coded_message.bin будет содержать байтовую последовательность, закодированную моей реализацией.