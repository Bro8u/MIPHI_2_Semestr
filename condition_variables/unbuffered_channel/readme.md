### Небуферизованный канал

В данном задании требуется реализовать небуферизованный канал, в который какие-то потоки могут класть данные, а другие потоки
могут оттуда их читать. В небуферизованном канале в один момент времени может храниться не более одного значения.
Если поток хочет положить в канал новое значение (вызывает метод `Put`), но в канале все еще лежит другое непрочитанное значение, то поток должен
быть заблокирован до того момента, когда старое значение прочитают.
После того как поток положил в канал значение, он блокируется до тех пор как другой поток прочитает положенное значение.
Если поток хочет прочитать значение (вызывает метод `Get`), но значения в канале еще нет, то поток должен быть заблокирован до того момента, пока
в канал не положат значение. Максимальное время ожидания может быть задано с помощью аргумента при вызове метода `Get`.
Если какой-то поток получил значение из канала, то ни этот поток ни другие потоки больше не смогут получить это же самое значение из канала.