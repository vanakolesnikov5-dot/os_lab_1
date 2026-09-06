#!/bin/bash

if [ $# -eq 0 ]; then
    echo "Ошибка: не переданы аргументы"
    echo "Использование: $0 число1 число2 число3 ..."
    exit 1
fi

count=0
sum=0

for num in "$@"; do
    sum=$((sum + num))
    count=$((count + 1))
done

average=$(echo "scale=2; $sum / $count" | bc 2>/dev/null || echo $((sum / count)))

echo "Количество аргументов: $count"
echo "Сумма: $sum"
echo "Среднее арифметическое: $average"
