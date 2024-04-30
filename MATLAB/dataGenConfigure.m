classdef dataGenConfigure < handle

    properties
        % N, M, T, P, D; 节点数量N，连边数量M，业务数量T，单边通道数量P、最大衰减距离D
        NMAX = 5000; NMIN = 2;
        MMAX = 5000; MMIN = 2;
        TMAX = 10000; TMIN = 2;
        PMAX = 80; PMIN = 2;
        DMAX = 1000; DMIN = 2;
        N = 1; M = 1; T = 1; P = 1; D = 1;  % 记录实际的NMTPD

        % 光网络的范围，一个cell的形状和大小，应该由外部决定，而不应该由编程者提供通用模板
        spaceLimX1 = 0; spaceLimX2 = 19;
        spaceLimY1 = 0; spaceLimY2 = 19;
        cellX = 1; cellY = 1; cellXGap = 1; cellYGap = 1;
        nNodeInCell = 4;
        nCell = 1;  % 交由Constructor初始化
        nodeAppearInCell = 0.4; % 在一个Cell中，每个Node的各自的出现的概率

        % 随机种子设置
        randomSeed = 1;

        % 对光网络环化的参数
        nPathCell2Cell = 3;    % 光网络的一个cell
        p_addPathCell2Cell = 1;   % 为相邻的Cell添加Path的概率
        nMaxNeighbor = 8;   % Cell的相邻Cell的个数
        connected_8 = false;    % 在环化时，是否往8连通方向进行搜索，否则4连通
        
        % 往光网络上添加光业务的参数
        p_distMode = [0.05, 0.3 0.35 0.2 0.1];
        num_distMode = 0;   % 交由Constructor初始化
        transactionPressure = 0.5; % 业务压力系数，控制着相同业务的重复次数，必须大于0小于1

        % 控制是否展示对数据进行可视化
        isVisualization = false;

        % 控制是否保存图片
        isSaveFigure = false; 

    end

    methods (Access = public)

        function this = dataGenConfigure()
            calculateNcell(this);
            generateD(this);
            generateP(this);
            this.num_distMode = length(this.p_distMode);
            if this.transactionPressure <= 0 || this.transactionPressure >= 1
                this.transactionPressure = 0.1;
            end
        end

    end

    methods (Access = private)

        function [] = calculateNcell(this)
            this.nCell = ((this.spaceLimX2 - this.spaceLimX1 - this.cellX) / (this.cellX + this.cellXGap) + 1) * ....
            ((this.spaceLimY2 - this.spaceLimY1 - this.cellY) / (this.cellY + this.cellYGap) + 1);
        end

        function [] = generateD(this)
            rng(this.randomSeed);
            this.D = randi(this.DMAX - this.DMIN + 1) + this.DMIN - 1;
        end

        function [] = generateP(this)
            rng(this.randomSeed);
            this.P = randi(this.PMAX - this.PMIN + 1) + this.PMIN - 1;
        end

        function [] = normalize_p_distMode(this)
            this.p_distMode = this.p_distMode / sum(this.p_distMode);
        end

    end

end