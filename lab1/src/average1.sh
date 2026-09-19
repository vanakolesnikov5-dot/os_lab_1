#!/bin/bash
sum=0
count=$#
for arg in "$@"
do
sum=$((sum+arg))
done
if [ $count -gt 0]; then
echo "кол-во аргументов: $count"
echo "среднее арифметическое; $((sum / count))"
else
echo "нет аргументов"
fi
