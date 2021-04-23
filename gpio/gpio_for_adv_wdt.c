/**
 *  * @author emlsyx
 *   * @email yangx_1118@163.com
 *    * @create date 2020-02-19 19:11:53
 *     * @modify date 2020-02-19 19:11:53
 *      * @desc [description]
 *       */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/poll.h>


#define USAGE_MESSAGE \
	    "Usage:\n" \
	        "  %s feed_time feed_pin wdt_en_pin en_flag   \n" \
		    "\n"

#define err_out(fmt, ...)                                                      \
	do                                                                         \
{                                                                          \
		printf(fmt, ##__VA_ARGS__);                                            \
		printf("<%s>  \"%s()\" %d error\n", __FILE__, __FUNCTION__, __LINE__); \
} while (0)

#define open_gpio_file(path, flag, fd)              \
	do                                              \
{                                               \
		fd = open(path, flag);                      \
		if (fd < 0)                                 \
		{                                           \
					err_out("\"%s\" open failed \n", path); \
					return (-1);                            \
				}                                           \
} while (0);

/**
 *  * @brief : gpio 导出
 *   * @param : pin
 *    * @retval: 0 成功; -1失败
 *     */
static int gpio_export(const int pin)
{
		int fd, len;
			char buffer[64];
				char path[64];

					sprintf(&path[0], "/sys/class/gpio/gpio%d", pin);
						/* 文件不存在时，导出gpio*/
						if (access(path, F_OK) != 0)
								{
											memset(path, 0, 64);
													sprintf(&path[0], "/sys/class/gpio/export");
															open_gpio_file(path, O_WRONLY, fd);
																	len = snprintf(buffer, sizeof(buffer), "%d", pin);

																			if (write(fd, buffer, len) < 0)
																						{
																										err_out("write failed to export gpio!\n");
																													return -1;
																															}
																					close(fd);
																						}
							return 0;
}
/**
 *  * @brief : gpio 卸载
 *   * @param : pin
 *    * @retval: 0 成功; -1失败
 *     */
static int gpio_unexport(const int pin)
{
		int fd, len;
			char buffer[64];
				char path[64];

					sprintf(&path[0], "/sys/class/gpio/gpio%d", pin);
						/* 文件存在时，卸载gpio*/
						if (access(path, F_OK) == 0)
								{
											memset(path, 0, 64);
													sprintf(&path[0], "/sys/class/gpio/unexport");
															open_gpio_file(path, O_WRONLY, fd);
																	len = snprintf(buffer, sizeof(buffer), "%d", pin);
																			if (write(fd, buffer, len) < 0)
																						{
																										err_out("write failed to unexport gpio!\n");
																													return -1;
																															}
																					close(fd);
																						}
							return 0;
}

/**
 *  * @brief : gpio 方向控制
 *   * @param : dir 方向(in/out)
 *    * @retval: 0 成功; -1失败
 *     */
static int gpio_direction(const int pin, const char *dir)
{
		int fd;
			char path[64];
				int w_len = ((dir[0] == 'i') && (dir[1] == 'n')) ? 2 : 3;

					snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", pin);
						open_gpio_file(path, O_WRONLY, fd);
							if (write(fd, dir, w_len) < 0)
									{
												err_out("Failed to set direction!\n");
														return -1;
															}

								close(fd);
									return 0;
}

/**
 *  * @brief : gpio 写
 *   * @param : 0 / 1
 *    * @retval: 0 成功; -1失败
 *     */
static int gpio_write(const int pin, const int value)
{
		int fd;
			char path[64];
				const char *val[] = {"0", "1"};

					snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", pin);
						open_gpio_file(path, O_WRONLY, fd);

							if (write(fd, val[value], 1) < 0)
									{
												err_out("Failed to write value!\n");
														return -1;
															}

								close(fd);
									return 0;
}

/**
 *  * @brief : gpio 读
 *   * @param : 读取的引脚值
 *    * @retval: 返回引脚的值 0 / 1
 *     */
static int gpio_read(const int pin)
{
		int fd;
			char path[64];
				char value_str[3];

					snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", pin);
						open_gpio_file(path, O_RDONLY, fd);
							if (read(fd, value_str, 3) < 0)
									{
												err_out("Failed to read value!\n");
														return -1;
															}

								close(fd);
									return (atoi(value_str));
}

/**
 *  * @brief : gpio 中断控制
 *   * @param : 0 none 表示引脚为输入，不是中断引脚
 *    *   @arg : 1 rising 表示引脚为中断输入，上升沿触发
 *     *   @arg : 2 falling 表示引脚为中断输入，下降沿触发
 *      *   @arg : 3 both 表示引脚为中断输入，边沿触发
 *       * @retval:
 *        */
static int gpio_edge(const int pin, int edge)
{
		int fd;
			char path[64];
				const char *dir_str[] = {"none", "rising", "falling", "both"};

					if (edge > 3)
							{
										err_out("Failed edge parameter error\n");
												return -1;
													}

						snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/edge", pin);
							open_gpio_file(path, O_WRONLY, fd);

								if (write(fd, dir_str[edge], strlen(dir_str[edge])) < 0)
										{
													err_out("Failed to set edge!\n");
															return -1;
																}

									close(fd);
										return 0;
}

int main(int argc, const char *argv[])
{

		if (argc != 5) {
					fprintf(stderr, USAGE_MESSAGE, argv[0]);
			return 0;
		}
			
			unsigned int feed_time = strtol(argv[1], NULL, 0);
				unsigned int feed_pin = strtol(argv[2], NULL, 0);
					unsigned int en_pin = strtol(argv[3], NULL, 0);
						unsigned int en_flag = strtol(argv[4], NULL, 0);

							/*WDT feed pin*/
							gpio_export(feed_pin);
								gpio_direction(feed_pin, "out");
									gpio_write(feed_pin, 0);

										/* WDT Enable pin*/
										gpio_export(en_pin);
											gpio_direction(en_pin, "out");
												gpio_write(en_pin, 1);

													if(en_flag == 0)
															{
																		printf("Disabled WDT\n");
																				return 1;
																					}
														else
															    	{
																			printf("Enabled WDT\n");
																					gpio_write(en_pin, 0);
																						}

															while (1)
																	{
																				sleep(feed_time);
																						gpio_write(feed_pin, 1);
																								sleep(feed_time);
																										gpio_write(feed_pin, 0);
																											}

																return 0;
}

