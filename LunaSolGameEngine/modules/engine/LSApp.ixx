export module Engine.LSApp;

export namespace LS
{
	class LSApp
	{
	public:
		virtual ~LSApp() = default;
		virtual void OnStart(int argc, char* argv[]) = 0;
		virtual int Run() = 0;
		virtual void Shutdown() = 0;

	protected:
		LSApp() = default;
	};
}