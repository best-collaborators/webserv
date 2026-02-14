#include <string>

class ChunkHandler
{
private:	
	std::string			_current_chunk;
	size_t				_expected_size;
	bool				_received;

public:
	ChunkHandler(/* args */);
	~ChunkHandler() = default;

	const std::string& 	 getChunk() const;
	size_t				 actualSize() const;
	size_t				 getExpectedSize() const;
	void				 setExpectedSize(size_t amount);
	void				 setChunk(std::string &&chunk);
	void				 markReceived();
	bool				 isReceived() const;
	void				 reset();
	void				 finalize();
	void				 append_to(std::string& body);
};

