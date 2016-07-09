/*
 * The Persper License
 *
 * Copyright (c) 2016 Persper Technologies Co., Ltd.
*/

#ifndef PERSPER_LIBPM_NAMING_H_
#define PERSPER_LIBPM_NAMING_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opens the persistent memory region from a backup file that
 * stores memory data. The file exists while the program is closed.
 * If the specified file does not exist, a new file is created and
 * the persistent memory region is empty.
 *
 * @param file The path to the backup file.
 * @param size The size of the persistent memory region. -1 lets it equal
 * the size of the backup file.
 *
*/
void pm_open(const char *file, size_t size);

/**
 * @brief Closes the persistent memory region and
 * saves data to the backup file.
 *
 * @return Zero on success. Otherwise, the caller should dump
 * persistent memory data in another way for safety.
*/
int pm_close();

/**
 * @brief Retrieves the location of a data object
 * in the persistent memory region.
 *
 * @param id The unique string ID of the data object.
 * @return The memory address of the data object.
*/
void *pm_retrieve(const char *id);

/**
 * @brief Registers a unique string ID for a data object.
 * After the ID is registered, it can be used to retrieve
 * the associated data object through pm_retrieve().
 * The registered pair holds no matter if the program is running or shut down.
 *
 * @param id The unique string ID of the data object.
 * @param addr The memory address of the data object.
*/
void pm_register(const char *id, void *addr);

/**
 * @brief Deregisters a unique ID, whose relationship with
 * an object in the persistent memory region is terminated.
*/
void pm_deregister(const char *id);

#ifdef __cplusplus
}
#endif

#endif // PERSPER_LIBPM_NAMING_H_
