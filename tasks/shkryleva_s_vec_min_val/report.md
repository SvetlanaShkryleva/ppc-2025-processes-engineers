# Отчет по лабораторной работе  
"Поиск минимального значения в векторе"

**Преподаватель**: Сысоев Александр Владимирович, доцент, программист 1 категории

---

## Введение
Параллельные вычисления являются важным направлением в современном программировании, позволяющим значительно ускорить обработку больших объемов данных. В данной работе рассматривается задача поиска минимального элемента в векторе с использованием технологии MPI (Message Passing Interface) для распределения вычислений между несколькими процессами.

---

## Постановка задачи
Разработать параллельный алгоритм для нахождения минимального значения в векторе целых чисел. Алгоритм должен эффективно распределять вычисления между несколькими процессами и обеспечивать корректный результат при различных размерах входных данных.

---

## Описание алгоритма

### Последовательный алгоритм
```cpp
int min_val = *std::min_element(GetInput().begin(), GetInput().end());
```
Алгоритм использует стандартную библиотечную функцию std::min_element для поиска минимального элемента, что обеспечивает временную сложность O(n).

### Параллельный алгоритм
Основные этапы параллельного алгоритма:

- Инициализация MPI - установка коммуникационной среды

- Распределение данных - разделение вектора между процессами

- Локальный поиск - каждый процесс находит минимум в своей части

- Глобальная редукция - сбор результатов со всех процессов

- Завершение работы - освобождение ресурсов MPI

---

## Описание схемы параллельного алгоритма

### Схема распределения данных
Процесс 0: [x₁, x₂, ..., xₖ] 
Процесс 1: [xₖ₊₁, xₖ₊₂, ..., x₂ₖ]
...
Процесс n-1: [xₘ₋ₖ, ..., xₘ]

### Коммуникационная схема
Broadcast - процесс 0 рассылает размер вектора всем процессам

Scatterv - распределение данных с учетом остатка от деления

Allreduce - сбор минимальных значений со всех процессов

### Особенности реализации
Использование MPI_Scatterv для неравномерного распределения

Применение MPI_Allreduce с операцией MPI_MIN для нахождения глобального минимума

Обработка краевых случаев (пустой вектор, один элемент)

---

## Описание программной реализации

### Ключевые компоненты:
```cpp

// Инициализация MPI
int initialized = 0;
MPI_Initialized(&initialized);
if (initialized == 0) {
  MPI_Init(nullptr, nullptr);
}

int world_rank = 0;
int world_size = 0;
MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
MPI_Comm_size(MPI_COMM_WORLD, &world_size);

// Распределение размера
int total_size = 0;
const std::vector<int> *input_data_ptr = nullptr;

if (world_rank == 0) {
  input_data_ptr = &GetInput();
  total_size = static_cast<int>(input_data_ptr->size());
}

MPI_Bcast(&total_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

// Расчет распределения данных
int base_local_size = total_size / world_size;
int remainder = total_size % world_size;

// Подготовка массивов для распределения данных
std::vector<int> sendcounts(world_size);
std::vector<int> displacements(world_size);

int offset = 0;
for (int i = 0; i < world_size; ++i) {
  sendcounts[i] = base_local_size + (i < remainder ? 1 : 0);
  displacements[i] = offset;
  offset += sendcounts[i];
}

// Создание локального буфера
std::vector<int> local_data(std::max(sendcounts[world_rank], 0));

// Распределение данных
MPI_Scatterv((world_rank == 0) ? input_data_ptr->data() : nullptr, 
             sendcounts.data(), displacements.data(), MPI_INT,
             local_data.data(), sendcounts[world_rank], MPI_INT, 
             0, MPI_COMM_WORLD);

// Локальный поиск минимума
int local_min = INT_MAX;
if (sendcounts[world_rank] > 0) {
  for (int value : local_data) {
    local_min = (value < local_min) ? value : local_min;
  }
}

// Глобальная редукция
int global_min = INT_MAX;
MPI_Allreduce(&local_min, &global_min, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);

GetOutput() = global_min;
```

### Особенности реализации
- Валидация: проверка корректности входных данных

- Предобработка: инициализация MPI и начальных значений

- Постобработка: корректное завершение работы MPI

---

## Результаты экспериментов

### Функциональное тестирование
Тестовые случаи:

- Малый вектор: {5, 3, 8, 1, 9} → ожидаемый результат: 1

- Вектор с отрицательными значениями: {-2, 0, 5, -8, 3} → -8

- Один элемент: {42} → 42

- Все элементы равны: {7, 7, 7, 7, 7} → 7

- Большие значения: {1000, -1000, 500, -500, 0} → -1000

Результаты: Все 10 тестов (5 для MPI + 5 для sequential) прошли успешно.

### Тестирование производительности
Параметры тестирования:

- Размер вектора: 100,000,000 элементов

- Диапазон значений: от -1000 до 1000

- Количество процессов: 1, 2, 4, 8

### Результаты времени выполнения (секунды):
| Версия     | Режим    | 1 процесс | 2 процесса | 4 процесса | 8 процессов |
|------------|----------|----------:|-----------:|-----------:|------------:|
| MPI        | pipeline | -         |  0.203753  |  0.151665  |  0.158276   |
| MPI        | task_run | -         |  0.205274  |  0.165121  |  0.158014   |
| Sequential | pipeline | 0.027252  | -          | -          | -           |
| Sequential | task_run | 0.027005  | -          | -          | -           |

### Анализ производительности:
- Замедление MPI: параллельная версия в 5.6-7.6 раз медленнее sequential эталона
- Стабильность MPI: Наблюдается улучшение от 2 к 4 процессам (~25% ускорение). При переходе к 8 процессам производительность стабилизируется
- Накладные расходы: время на коммуникации составляет значительную часть общего времени выполнения

### Подтверждение корректности:
- Все функциональные тесты пройдены
- Результаты MPI и sequential версий идентичны

---

## Выводы из результатов
- Функциональная корректность: все 10 тестов успешно пройдены, алгоритмы работают правильно
- Ограничения параллелизации: для задачи размером 100,000,000 элементов накладные расходы MPI делают параллелизацию неэффективной
- Относительная эффективность: наилучший результат достигнут при 4 процессах в режиме pipeline
- Область применения MPI: может стать эффективной при значительном увеличении объема данных или усложнении вычислений
- Масштабируемость: алгоритм демонстрирует стабильную работу с различным количеством процессов

---

## Заключение
В ходе работы успешно разработаны и реализованы последовательный и параллельный алгоритмы поиска 
минимального значения в векторе. Функциональное тестирование подтвердило корректность работы 
обоих алгоритмов для различных тестовых случаев.

Анализ производительности показал, что для текущего размера задачи (100,000,000 элементов) 
параллельная MPI реализация не обеспечивает ускорения относительно sequential подхода. 
Накладные расходы на инициализацию MPI и межпроцессные коммуникации превышают вычислительную 
выгоду от распараллеливания. Наилучший результат MPI достигнут при 4 процессах в режиме pipeline, 
где время выполнения составило 0.151665 с. против эталонного sequential времени 0.027252 с.

Несмотря на отсутствие абсолютного ускорения, работа демонстрирует корректную реализацию 
параллельного алгоритма и понимание принципов распределенных вычислений. Для демонстрации 
реальных преимуществ MPI требуются задачи с большим объемом вычислений или данные большего размера.

---

## Список литературы
MPI Standard https://www.mpi-forum.org/docs/
MPICH guides: https://www.mpich.org/documentation/guides/
Microsoft MPI: https://www.learn.microsoft.com/en-us/message-passing-interface/microsoft-mpi
OpenMPI docs: https://www.open-mpi.org/docs/

---

## Приложение
```cpp
bool ShkrylevaSVecMinValMPI::RunImpl() {
  if (GetInput().empty()) {
    return false;
  }

  int world_rank = 0;
  int world_size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  int total_size = 0;
  const std::vector<int> *input_data_ptr = nullptr;

  if (world_rank == 0) {
    input_data_ptr = &GetInput();
    total_size = static_cast<int>(input_data_ptr->size());
  }

  MPI_Bcast(&total_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (total_size == 0) {
    return false;
  }

  int base_local_size = total_size / world_size;
  int remainder = total_size % world_size;

  std::vector<int> sendcounts(world_size);
  std::vector<int> displacements(world_size);

  int offset = 0;
  for (int i = 0; i < world_size; ++i) {
    sendcounts[i] = base_local_size + (i < remainder ? 1 : 0);
    displacements[i] = offset;
    offset += sendcounts[i];
  }

  std::vector<int> local_data(std::max(sendcounts[world_rank], 0));

  MPI_Scatterv((world_rank == 0) ? input_data_ptr->data() : nullptr, sendcounts.data(), displacements.data(), MPI_INT,
               local_data.data(), sendcounts[world_rank], MPI_INT, 0, MPI_COMM_WORLD);

  int local_min = INT_MAX;
  if (sendcounts[world_rank] > 0) {
    for (int value : local_data) {
      local_min = (value < local_min) ? value : local_min;
    }
  }

  int global_min = INT_MAX;
  MPI_Allreduce(&local_min, &global_min, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);

  GetOutput() = global_min;

  return true;
}

bool ShkrylevaSVecMinValMPI::PostProcessingImpl() {
  int finalized = 0;
  MPI_Finalized(&finalized);
  if (finalized == 0) {
    // MPI_Finalize();
  }
  return GetOutput() > INT_MIN;
}
```
