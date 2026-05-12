NAME		:=ircserv

CXX			:=c++
STD			:=-std=c++20
WFLAGS		:=-Wall -Wextra -Werror -Wpedantic -Wunreachable-code -Wshadow \
			-Wnull-dereference -Wfloat-equal -Wcast-align -Wformat=2 \
			-Wswitch-enum -Wundef -Wconversion -Wsign-conversion \
			-Wextra-semi -Wcomma -Wparentheses -Wimplicit-fallthrough \
			-Wnon-virtual-dtor -Wctor-dtor-privacy -Wtype-limits

DEFS		:=
OPTS		:=-O2 -DNDEBUG
DEBUG		:=-O0 -g -fsanitize=address,undefined,alignment,float-cast-overflow,float-divide-by-zero -fno-omit-frame-pointer
LDFLAGS		:=

CXXFLAGS	:=$(STD) $(WFLAGS) $(DEFS) $(OPTS)
ifeq ($(MAKELEVEL),0)
	MAKEFLAGS	+= --no-print-directory -j$(shell nproc 2>/dev/null || echo 4)
endif

DIR_INC		:=inc/
DIR_SRC		:=src/
DIR_LIB		:=lib/
DIR_OBJ		:=obj/
DIR_DEP		:=dep/
DIR_UTILS	:=utils/

SRC_FILES	:=main.cpp Server.cpp Client.cpp CommandRequest.cpp
UTILS_FILES	:=

INCS		:=$(addprefix -I, $(DIR_INC) $(DIR_UTILS))
SRCS		:=$(addprefix $(DIR_SRC), $(SRC_FILES)) $(addprefix $(DIR_UTILS), $(UTILS_FILES))
OBJS		:=$(addprefix $(DIR_OBJ), $(SRC_FILES:.cpp=.o) $(UTILS_FILES:.cpp=.o))
DEPS		:=$(patsubst $(DIR_OBJ)%.o, $(DIR_DEP)%.d, $(OBJS))

BLUE		:=\033[1;34m
YELLOW		:=\033[1;33m
GREEN		:=\033[1;32m
RED			:=\033[1;31m
COLOR		:=\033[0m


all: $(NAME)

$(NAME): $(OBJS)
	@$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS)
	$(call output)

$(DIR_OBJ)%.o: $(DIR_SRC)%.cpp
	$(call compile_objs)

$(DIR_OBJ)%.o: $(DIR_UTILS)%.cpp
	$(call compile_objs)

clean:
	$(call rm_dir,$(DIR_OBJ))

fclean: clean
	$(call rm_dir,$(DIR_LIB))
	$(call rm_dir,$(DIR_DEP))
	$(call rm_file,$(NAME))

re:
	@$(MAKE) fclean
	@$(MAKE) all

debug:
	@$(MAKE) fclean
	@$(MAKE) all OPTS="$(DEBUG)"

.PHONY: all clean fclean re debug
.SECONDARY: $(OBJS) $(DEPS)

-include $(DEPS)

define compile_objs
	@mkdir -p $(dir $@) $(patsubst $(DIR_OBJ)%, $(DIR_DEP)%, $(dir $@))
	@$(CXX) $(CXXFLAGS) -c $< -o $@ -MMD -MP -MF $(patsubst $(DIR_OBJ)%.o, $(DIR_DEP)%.d, $@) $(INCS)
	@echo "$(GREEN) [+]$(COLOR) compiling $@"
endef

define rm_dir
	@if [ -d "$(1)" ]; then \
		rm -rf $(1); \
		echo "$(RED) [-]$(COLOR) removed $(1)"; \
	fi
endef

define rm_file
	@if [ -e "$(1)" ]; then \
		rm -f $(1); \
		echo "$(RED) [-]$(COLOR) removed $(1)"; \
	fi
endef

define output
	@echo "$(YELLOW) [✔] $(NAME) created$(COLOR)"
	@echo "$(GREEN) [/] usage: $(YELLOW)./$(NAME) <port> <password>$(COLOR)";
endef
