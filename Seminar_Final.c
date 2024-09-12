#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("");
MODULE_DESCRIPTION("Tic-Tac-Toe Kernel Module");

#define TTT_BUF_SIZE 9 // The board has 9 spaces

#define IOCTL_SET_CMD _IOW('T', 1, struct game_command)
#define IOCTL_GET_CMD _IOR('T', 2, struct game_command)

struct game_command {
    int command;
    int value;
};

static char ttt_board[TTT_BUF_SIZE];
static int isReg = 0;
static int major;
static int ttt_size;

static int ttt_init(void);
static void ttt_exit(void);
static ssize_t ttt_read(struct file*, char*, size_t, loff_t*);
static ssize_t ttt_write(struct file*, const char*, size_t, loff_t*);
static long ttt_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

static struct file_operations ttt_fops = {
    .read = ttt_read,
    .write = ttt_write,
    .unlocked_ioctl = ttt_ioctl,
};

static long ttt_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    struct game_command cmd_data;

    switch (cmd) {
        case IOCTL_SET_CMD:
            if (copy_from_user(&cmd_data, (struct game_command *)arg, sizeof(struct game_command))) {
                return -1;
            }
            // Handle the command (update board)
            printk(KERN_INFO "Player %d moved to position %d\n", cmd_data.value, cmd_data.command);
            break;

        case IOCTL_GET_CMD:
            // This could return the current game state if necessary
            if (copy_to_user((struct game_command *)arg, &cmd_data, sizeof(struct game_command))) {
                return -1;
            }
            break;

        default:
            return -1; // Command not supported
    }
    return 0;
}

int ttt_init(void) {
    major = register_chrdev(0, "Seminar_Final", &ttt_fops);
    memset(ttt_board, '-', TTT_BUF_SIZE); // Initialize the board with empty slots
    ttt_size = 0;

    if(major < 0) {
        isReg = 0;
        printk(KERN_INFO "TicTacToe: Initialization failed\n");
    } else {
        isReg = 1;
        printk(KERN_INFO "TicTacToe: Initialization successful\n");
    }

    return 0;
}

void ttt_exit(void) {
    if(isReg) {
        unregister_chrdev(major, "Seminar_Final");
    }
    printk(KERN_INFO "TicTacToe: Exiting module\n");
}

static ssize_t ttt_read(struct file *fp, char *buf, size_t count, loff_t *pos)
{
    int error_count = 0;
    if (*pos >= TTT_BUF_SIZE)
        return 0; // End of board

    if (*pos + count > TTT_BUF_SIZE)
        count = TTT_BUF_SIZE - *pos;

    error_count = copy_to_user(buf, ttt_board + *pos, count);
    if (error_count == 0) {
        *pos += count;
        return count;
    } else {
        printk(KERN_INFO "TicTacToe: Failed to send %d characters to the user\n", error_count);
        return -1;
    }
}

static ssize_t ttt_write(struct file *fp, const char *buf, size_t count, loff_t *pos)
{
    int position;
    char move;
    if (count < 2 || *pos > TTT_BUF_SIZE - 1 || *pos < 0) {
        printk(KERN_INFO "TicTacToe: Invalid move\n");
        return -1;
    }

    if (copy_from_user(&move, buf, 1)) {
        printk(KERN_INFO "TicTacToe: Error copying from user\n");
        return -1;
    }

    position = (int)(*pos);

    if (ttt_board[position] == '-') { // Check if the position is empty
        ttt_board[position] = move; // Place the player's move
        printk(KERN_INFO "TicTacToe: Player %c moved to position %d\n", move, position);
        return 1; // Move was successful
    } else {
        printk(KERN_INFO "TicTacToe: Position %d already filled\n", position);
        return -1; // Position already occupied
    }
}

module_init(ttt_init);
module_exit(ttt_exit);