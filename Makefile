NAME = webserv

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -MMD -MP -std=c++17 -Iincludes -Iincludes/http-request-parser -Iincludes/sockets

SRC_DIR = sources
OBJ_DIR = build

SRCS = \
	$(SRC_DIR)/http-request-parser/HttpStatus.cpp \
	$(SRC_DIR)/http-request-parser/Response.cpp \
	$(SRC_DIR)/http-request-parser/RequestGenerator.cpp \
	$(SRC_DIR)/http-request-parser/Trimmer.cpp \
	$(SRC_DIR)/http-request-parser/MultipartFormData.cpp \
	$(SRC_DIR)/http-request-parser/MultipartDataValidator.cpp \
	$(SRC_DIR)/http-request-parser/ValidatorHelpers.cpp \
	$(SRC_DIR)/http-request-parser/RequestParseResult.cpp \
	$(SRC_DIR)/http-request-parser/RequestParser.cpp \
	$(SRC_DIR)/sockets/main.cpp \
	$(SRC_DIR)/sockets/Socket.cpp \
	$(SRC_DIR)/sockets/Poller.cpp \
	$(SRC_DIR)/sockets/Connection.cpp \
	$(SRC_DIR)/sockets/Listener.cpp \
	$(SRC_DIR)/sockets/Server.cpp

OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEPS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.d)

all: $(OBJ_DIR) $(NAME)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

-include $(DEPS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

debug: CXXFLAGS += -DDEBUG
debug: re

.PHONY: all clean fclean re debug