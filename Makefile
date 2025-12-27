NAME = webserv

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -MMD -MP -std=c++17 -Iincludes -Iincludes/http-request-parser

SRC_DIR = sources
OBJ_DIR = build

SRCS = \
	$(SRC_DIR)/http-request-parser/main.cpp \
	$(SRC_DIR)/http-request-parser/HttpStatus.cpp \
	$(SRC_DIR)/http-request-parser/ResponseGenerator.cpp \
	$(SRC_DIR)/http-request-parser/RequestParser.cpp

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