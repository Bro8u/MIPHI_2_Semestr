Домашние задачи для курса МИФИ "Введение в программирование, часть 2", 2024 год

## Настройка окружения

**Linux**

Для локального тестирования задач потребуется g++ компилятор и makefile тулза.

Мы рекомендуем использовать Ubuntu версии **20.04**. Дополнительных действий по настройке в таком случае не должно требоваться.

**Windows**

Для локального тестирования достаточно собрать проект, состоящий из всех файлов в папке, и запустить скомпилированный main. В этом может помочь IDE CLion или VSCode.

Однако в тестирующей системе тесты запускаются иначе. Используется ОС Linux, g++ компилятор и makefile тулза. Если есть желание воспроизвести запуск как в тестирующей системе, можно воспользоваться [WSL](https://www.jetbrains.com/help/clion/how-to-use-wsl-development-environment-in-clion.html). Гайд для установки WSL: https://gitlab.manytask.org/mephi-dap-programming-2/public-2024/-/blob/main/docs/wsl.pdf

## Клонируем репозиторий

```bash
# Переходим в директорию, где вы хотите разместить репозиторий с задачами.
# Например, `/home/<username>/mephi/programming-2` для Linux или `/Users/<username>/mephi/programming-2` для MacOS.
$ cd <выбранная директория>

# Клонируем свой репозиторий, который создается автоматически при регистрации на manytask.
# Имя репозитория доступно по ссылке "MY REPO" на mephi-dap.manytask.org.
$ git clone https://gitlab.manytask.org/mephi-dap-programming-2/students-2024/<ваш login с mephi-dap.manytask.org>.git

# Переходим в директорию склонированного репозитория.
$ cd <ваш репозиторий>

# Настраиваем свое имя в git.
$ git config --local user.name "<ваш login с mephi-dap.manytask.org>"
$ git config --local user.email "<ваш email с mephi-dap.manytask.org>"

# По дефолту просим git запускать команду rebase для подтягивания конфликтующих изменений.
$ git config --local pull.rebase true

# Настраиваем возможность получать обновления при появлении новых задач.
$ git remote add upstream https://gitlab.manytask.org/mephi-dap-programming-2/public-2024.git
```

## Подтягиваем новые задачи

Периодически необходимо подтягивать изменения из основного репозитория с задачками: https://gitlab.manytask.org/mephi-dap-programming-2/public-2024. Например, когда появляются новые задачки или меняются условия старых.

```bash
$ git pull upstream main
```

Если старые задачи менялись, при подтягивании изменений возможен конфликт ваших локальных изменений и новых изменений от преподавателей. Для разрешения конфликта предлагается:
```bash
# Показать файлы с конфликтами.
$ git status

$ ... разрешение конфликта в каждом из файлов ...

# Сообщаем git для каждого файла, что конфликт разрешен.
$ git add <файл с конфликтом>

# Продолжаем подтягивание изменений.
# После выполнения команды последующие изменения могут опять создать конфликт и процедуру разрешения конфликта придется повторить.
$ git rebase --continue
```

Когда убедились, что изменения корректно применились, пушим их в репозиторий:
```bash
$ git push -f
```

Если послать решение задачи вместе с изменениями от rebase, то тестирующая система не сможет определить список решенных задач, потому что rebase полностью переписывает историю. В такой ситуации предлагается поменять свое решение любым образом (например, добавив лишнюю пустую строку) и послать решение еще раз.

## Решаем задачи

В каждой из папок находится условие и файлы очередной задачи.

Файлы main.cpp и makefile в каждой задаче отвечают за тестирование. При проверке решения они перезаписываются, так что их изменения не учитываются.

Остальные файлы менять можно и нужно.

Для тестирования в каждой задаче достаточно вызвать:
```bash
$ make
```

Команда соберет бинарные файлы из исходных и запустит тесты в main.cpp.

Как только убедились, что решение работает, отправить его можно коммитом в репозиторний:
```bash
$ git add <измененный файл>
$ git commit -m "<комментарий, описывающий изменения>"
$ git push origin main
```

Процесс проверки автоматизирован, результаты проверки можно увидеть либо в CI/CD коммита (зеленая/красная/желтая галочка напротив коммита), либо во вкладках SUBMITS и GDOC главной страницы курса https://mephi-dap.manytask.org/.

Закрытых тестов в задачах нет, все тесты публичные и доступны в main.cpp. Сообщения, записанные в консоль в процессе тестирования, можно увидеть в результатах проверки.

## Codestyle
Для того, чтобы пройти ревью, нужно, чтобы код соответсвовал следующему кодстайлу: https://ysda.gitlab.io/algorithms/styleguide.html
