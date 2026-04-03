INC_DIR = includes
PARSER_DIR = http-request-parser
CGI_DIR = cgi
EXECUTION_DIR = execution

INCL = \
	-I$(INC_DIR)/$(PARSER_DIR) \
	-I$(INC_DIR)/$(PARSER_DIR)/request \
	-I$(INC_DIR)/$(PARSER_DIR)/request-validation \
	-I$(INC_DIR)/$(PARSER_DIR)/configuration-file \
	-I$(INC_DIR)/$(PARSER_DIR)/response \
	-I$(INC_DIR)/$(EXECUTION_DIR) \
	-I$(INC_DIR)/$(CGI_DIR)

NAME = webserv

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -MMD -MP -std=c++17 -Iincludes $(INCL)

SRC_DIR = sources
OBJ_DIR = build

SRCS = \
	$(SRC_DIR)/$(PARSER_DIR)/configuration-file/ConfigurationFileParser.cpp \
	$(SRC_DIR)/$(PARSER_DIR)/configuration-file/ListenData.cpp \
	$(SRC_DIR)/$(PARSER_DIR)/RequestGenerator.cpp \
	$(SRC_DIR)/$(PARSER_DIR)/RequestParser.cpp \
	$(SRC_DIR)/$(PARSER_DIR)/FileUploadHandler.cpp \
	$(SRC_DIR)/$(PARSER_DIR)/RequestLineValidator.cpp \
	$(SRC_DIR)/$(PARSER_DIR)/TransferEncodingChunked.cpp \
	$(SRC_DIR)/$(PARSER_DIR)/RequestStringUtils.cpp \
	$(SRC_DIR)/$(PARSER_DIR)/PercentEncoder.cpp \
	$(SRC_DIR)/$(PARSER_DIR)/HttpBodyParser.cpp \
	$(SRC_DIR)/$(PARSER_DIR)/HttpHeaderParser.cpp \
	$(SRC_DIR)/$(PARSER_DIR)/HttpContentType.cpp \
	$(SRC_DIR)/$(PARSER_DIR)/File.cpp \
	$(SRC_DIR)/$(PARSER_DIR)/HttpStatus.cpp \
	$(SRC_DIR)/$(PARSER_DIR)/HttpMethod.cpp \
	$(SRC_DIR)/$(PARSER_DIR)/HttpMethodRegistry.cpp \
	$(SRC_DIR)/$(PARSER_DIR)/HttpRegexPatterns.cpp \
	$(SRC_DIR)/$(PARSER_DIR)/RegexMatcher.cpp \
	$(SRC_DIR)/$(PARSER_DIR)/Response.cpp \
	$(SRC_DIR)/$(PARSER_DIR)/Trimmer.cpp \
	$(SRC_DIR)/$(PARSER_DIR)/MultipartFormData.cpp \
	$(SRC_DIR)/$(PARSER_DIR)/MultipartDataParser.cpp \
	$(SRC_DIR)/$(PARSER_DIR)/HttpMessage.cpp \
	$(SRC_DIR)/$(PARSER_DIR)/Request.cpp \
	$(SRC_DIR)/$(PARSER_DIR)/ChunkHandler.cpp \
	$(SRC_DIR)/$(PARSER_DIR)/HttpRequestReader.cpp \
	$(SRC_DIR)/$(PARSER_DIR)/HttpResponseWriter.cpp \
	$(SRC_DIR)/$(PARSER_DIR)/BufferManager.cpp \
	$(SRC_DIR)/$(PARSER_DIR)/ServerBlock.cpp \
	$(SRC_DIR)/$(PARSER_DIR)/Location.cpp \
	$(SRC_DIR)/$(PARSER_DIR)/ListingGenerator.cpp \
	$(SRC_DIR)/Logger.cpp \
	$(SRC_DIR)/$(EXECUTION_DIR)/main.cpp \
	$(SRC_DIR)/$(EXECUTION_DIR)/Socket.cpp \
	$(SRC_DIR)/$(EXECUTION_DIR)/Poller.cpp \
	$(SRC_DIR)/$(EXECUTION_DIR)/Connection.cpp \
	$(SRC_DIR)/$(EXECUTION_DIR)/Listener.cpp \
	$(SRC_DIR)/$(EXECUTION_DIR)/Server.cpp \
	$(SRC_DIR)/$(EXECUTION_DIR)/PipeFD.cpp \
	$(SRC_DIR)/$(EXECUTION_DIR)/ChildSignalHandler.cpp \
	$(SRC_DIR)/$(CGI_DIR)/CGIRequestConfig.cpp \
	$(SRC_DIR)/$(CGI_DIR)/CGIHandler.cpp \
	$(SRC_DIR)/$(CGI_DIR)/CGIValidator.cpp \
	$(SRC_DIR)/$(CGI_DIR)/CGIExecutor.cpp \
	$(SRC_DIR)/$(CGI_DIR)/CGIPath.cpp

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

debug: CXXFLAGS += -DDEBUG_FLAG -g -O0
debug: re

memory: CXXFLAGS += -fsanitize=address -g

.PHONY: all clean fclean re debug memory