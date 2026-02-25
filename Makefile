INC_DIR = includes

INCL = \
	-I$(INC_DIR)/http-request-parser \
	-I$(INC_DIR)/http-request-parser/request \
	-I$(INC_DIR)/http-request-parser/request-validation \
	-I$(INC_DIR)/http-request-parser/configuration-file \
	-I$(INC_DIR)/http-request-parser/response \
	-I$(INC_DIR)/sockets

NAME = webserv

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -MMD -MP -std=c++17 -Iincludes $(INCL)

SRC_DIR = sources
OBJ_DIR = build

SRCS = \
	$(SRC_DIR)/http-request-parser/configuration-file/ConfigurationFileParser.cpp \
	$(SRC_DIR)/http-request-parser/RequestGenerator.cpp \
	$(SRC_DIR)/http-request-parser/RequestParser.cpp \
	$(SRC_DIR)/http-request-parser/FileUploadHandler.cpp \
	$(SRC_DIR)/http-request-parser/RequestLineValidator.cpp \
	$(SRC_DIR)/http-request-parser/TransferEncodingChunked.cpp \
	$(SRC_DIR)/http-request-parser/RequestStringUtils.cpp \
	$(SRC_DIR)/http-request-parser/PercentEncoder.cpp \
	$(SRC_DIR)/http-request-parser/HttpBodyParser.cpp \
	$(SRC_DIR)/http-request-parser/HttpHeaderParser.cpp \
	$(SRC_DIR)/http-request-parser/HttpContentType.cpp \
	$(SRC_DIR)/http-request-parser/HttpStatus.cpp \
	$(SRC_DIR)/http-request-parser/HttpMethod.cpp \
	$(SRC_DIR)/http-request-parser/HttpMethodRegistry.cpp \
	$(SRC_DIR)/http-request-parser/HttpRegexPatterns.cpp \
	$(SRC_DIR)/http-request-parser/RegexMatcher.cpp \
	$(SRC_DIR)/http-request-parser/Response.cpp \
	$(SRC_DIR)/http-request-parser/Trimmer.cpp \
	$(SRC_DIR)/http-request-parser/MultipartFormData.cpp \
	$(SRC_DIR)/http-request-parser/MultipartDataParser.cpp \
	$(SRC_DIR)/http-request-parser/HttpMessage.cpp \
	$(SRC_DIR)/http-request-parser/Request.cpp \
	$(SRC_DIR)/http-request-parser/ChunkHandler.cpp \
	$(SRC_DIR)/http-request-parser/HttpRequestReader.cpp \
	$(SRC_DIR)/http-request-parser/HttpResponseWriter.cpp \
	$(SRC_DIR)/http-request-parser/BufferManager.cpp \
	$(SRC_DIR)/Logger.cpp \
	$(SRC_DIR)/sockets/main.cpp \
	$(SRC_DIR)/sockets/Socket.cpp \
	$(SRC_DIR)/sockets/Poller.cpp \
	$(SRC_DIR)/sockets/Connection.cpp \
	$(SRC_DIR)/sockets/Listener.cpp \
	$(SRC_DIR)/sockets/Server.cpp \
	$(SRC_DIR)/sockets/PipeFD.cpp \
	$(SRC_DIR)/sockets/ChildSignalHandler.cpp \
	$(SRC_DIR)/sockets/CGIRequestConfig.cpp \
	$(SRC_DIR)/sockets/CGIHandler.cpp \
	$(SRC_DIR)/sockets/CGIExecutor.cpp

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

debug: CXXFLAGS += -DDEBUG_FLAG
debug: re

.PHONY: all clean fclean re debug